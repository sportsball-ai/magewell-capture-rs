#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/select.h>

#include "ProductVer.h"
#include "LibMWCapture/MWCapture.h"
#include "MWFOURCC.h"

#include "OpenGLRender/OpenGLRender.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "Text.h"
#define MAX_CAPTURE 16
#define VIDEO_FRAME_NUM 3




#define CAPTURE_FRAME_RATE 25
#define CAPTURE_WIDTH 1920
#define CAPTURE_HEIGHT 1080
#define CAPTURE_FOURCC MWFOURCC_YUY2
#define RENDER_FOURCC RENDER_YUY2

static int g_image_size = 0;
static int g_min_stride = 0;

static unsigned char *g_video_frame[VIDEO_FRAME_NUM];
static int g_vfw_index = 0;


static int g_video_capture = 0;
static int g_running = 0;


static int g_capture_num = 0;
static char g_capture_name[MAX_CAPTURE][80];
static int g_capture_index[MAX_CAPTURE];
static HCHANNEL g_channel_handle = NULL;

static bool g_border = 0;
static bool g_text = 1;
static unsigned char *image = NULL;

void print_version()
{
    BYTE maj, min;
    WORD build;
    MWGetVersion(&maj, &min, &build);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - OSDPreview\n", maj, min, build);
    printf("Only pro devices are supported\n");
}
void load_osd(HOSD osd)
{
    int i;
    static bool prev_text = 0;
    static bool prev_border = 1;
    if(g_border != prev_border){
        memset(image, 0, CAPTURE_WIDTH*CAPTURE_HEIGHT*4);
        if(g_border){
            memset(image + 40*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + 41*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + 42*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + 42*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + (CAPTURE_HEIGHT - 8)*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + (CAPTURE_HEIGHT - 7)*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + (CAPTURE_HEIGHT - 6)*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            memset(image + (CAPTURE_HEIGHT - 5)*CAPTURE_WIDTH*4 + 16, 255, (CAPTURE_WIDTH - 10) * 4);
            for(i = 43; i < (CAPTURE_HEIGHT-8); i++){
                memset(image+CAPTURE_WIDTH*4*i+16,255,16);
                memset(image+CAPTURE_WIDTH*4*i+(CAPTURE_WIDTH - 10) * 4,255,16);
            }
        }
        MWUploadImageFromVirtualAddress(g_channel_handle,
            osd, MWCAP_VIDEO_COLOR_FORMAT_RGB, 
            MWCAP_VIDEO_QUANTIZATION_FULL, MWCAP_VIDEO_SATURATION_FULL,
            0, 0, CAPTURE_WIDTH, CAPTURE_HEIGHT, 
            (MWCAP_PTR64)image, CAPTURE_WIDTH*CAPTURE_WIDTH*4, CAPTURE_WIDTH*4, CAPTURE_WIDTH, CAPTURE_HEIGHT, 
            FALSE, TRUE, TRUE);
    }
    prev_border = g_border;
    if(g_text){
        int w,h;
        char text[32];
        time_t timep;   
        struct tm *p; 
        time(&timep);   
        p = localtime(&timep);
        sprintf(text, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);
        h=16;
        w=8*8;
        memset(image, 0, w*h*4);
        add_text(image, text, 0, 0, w, h);
        MWUploadImageFromVirtualAddress(g_channel_handle,
            osd, MWCAP_VIDEO_COLOR_FORMAT_RGB, 
            MWCAP_VIDEO_QUANTIZATION_FULL, MWCAP_VIDEO_SATURATION_FULL,
            100, 100, 4*w, 4*h, 
            (MWCAP_PTR64)image, w*h*4, w*4, w, h, 
            FALSE, TRUE, TRUE);
    }
    else if(prev_text){
        int w,h;
        h=16;
        w=8*8;
        memset(image, 0, w*h*4);
        MWUploadImageFromVirtualAddress(g_channel_handle,
            osd, MWCAP_VIDEO_COLOR_FORMAT_RGB, 
            MWCAP_VIDEO_QUANTIZATION_FULL, MWCAP_VIDEO_SATURATION_FULL,
            100, 100, 4*w, 4*h, 
            (MWCAP_PTR64)image, w*h*4, w*4, w, h, 
            FALSE, TRUE, TRUE);
    }
    prev_text = g_text;
}
void *video_capture(void *parm)
{
    HNOTIFY notify = 0;
    bool capture_start = 0;
    MWCAP_PTR notify_event = 0;
    MWCAP_PTR capture_event = 0;
    MWCAP_VIDEO_BUFFER_INFO video_buffer_info;
    MWCAP_VIDEO_FRAME_INFO video_frame_info;
    MWCAP_VIDEO_SIGNAL_STATUS video_signal_status;
    DWORD notify_buffer_mode = MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED;
    RECT osd_rects[] = { { 0, 0, CAPTURE_WIDTH, CAPTURE_HEIGHT} };
    HOSD osd = MWCreateImage(g_channel_handle, CAPTURE_WIDTH, CAPTURE_HEIGHT);
    if(NULL == osd){
        printf("create osd fail\n");
        goto video_capture_stoped;
    }
    //HCHANNEL g_channel_handle = (HCHANNEL)parm;
    notify_event = MWCreateEvent();
    if(NULL == notify_event){
        printf("create notify_event fail\n");
        goto video_capture_stoped;
    }
    capture_event = MWCreateEvent();
    if(NULL == capture_event){
        printf("create capture_event fail\n");
        goto video_capture_stoped;
    }
    if (MW_SUCCEEDED != MWStartVideoCapture(g_channel_handle, capture_event)) {
        printf("start video capture fail!\n");
        goto video_capture_stoped;
    }
    capture_start = 1;
    if(MW_SUCCEEDED != MWGetVideoSignalStatus(g_channel_handle, &video_signal_status)){
        printf("can't get video signal status\n");
        goto video_capture_stoped;
    }
    switch (video_signal_status.state) {
    case MWCAP_VIDEO_SIGNAL_NONE:
        printf("input signal status: NONE\n");
        break;
    case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
        printf("input signal status: Unsupported\n");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKING:
        printf("input signal status: Locking\n");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKED:
        printf("input signal status: Locked\n");
        break;
    default:
        printf("input signal status: unknow %d\n",video_signal_status.state);
    }
    if (video_signal_status.state != MWCAP_VIDEO_SIGNAL_LOCKED) {
        printf("input signal status error\n");
        goto video_capture_stoped;
    }
    if(video_signal_status.bInterlaced){
        notify_buffer_mode = MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED;
    }
    notify= MWRegisterNotify(g_channel_handle, notify_event, notify_buffer_mode);
    if (notify == 0) {
        printf("Register Notify fail\n");
        goto video_capture_stoped;
    }
    while(g_video_capture && g_running){
        MW_RESULT ret;
        ULONGLONG notify_status = 0;
        MWCAP_VIDEO_DEINTERLACE_MODE mode;
        if(MWWaitEvent(notify_event, 1000) <= 0) {
            printf("wait notify error or timeout\n");
            break;
        }
        if (MWGetNotifyStatus(g_channel_handle, notify, &notify_status) != MW_SUCCEEDED){
            continue;
        }
        if (MWGetVideoBufferInfo(g_channel_handle, &video_buffer_info) != MW_SUCCEEDED){
            continue;
        }
        if (MWGetVideoFrameInfo(g_channel_handle, video_buffer_info.iNewestBufferedFullFrame, &video_frame_info) != MW_SUCCEEDED){
            continue;
        }
        if (0 == (notify_status & notify_buffer_mode)) {
            continue;
        }
        if(video_signal_status.bInterlaced){
            if(0 == video_buffer_info.iBufferedFieldIndex){
                mode = MWCAP_VIDEO_DEINTERLACE_TOP_FIELD;
            }else{
                mode = MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD;
            }
        }else{
            mode = MWCAP_VIDEO_DEINTERLACE_BLEND;
        }
        load_osd(osd);
        ret = MWCaptureVideoFrameWithOSDToVirtualAddress(g_channel_handle,
            MWCAP_VIDEO_FRAME_ID_NEWEST_BUFFERED, 
            (MWCAP_PTR)g_video_frame[g_vfw_index],
            g_image_size,
            g_min_stride,
            FALSE,
            NULL,
            CAPTURE_FOURCC,
            CAPTURE_WIDTH,
            CAPTURE_HEIGHT,
            osd,
            osd_rects,
            1);
        if(MW_SUCCEEDED != ret){
            continue;
        }
        if(MWWaitEvent(capture_event, 1000) <= 0) {
            printf("wait capture event error or timeout\n");
            break;
        }
        MWCAP_VIDEO_CAPTURE_STATUS captureStatus;
        MWGetVideoCaptureStatus(g_channel_handle, &captureStatus);
        g_vfw_index++;
        if(g_vfw_index >= VIDEO_FRAME_NUM){
            g_vfw_index = 0;
        }
    }
video_capture_stoped:
    g_video_capture = 0;
    if(osd){
        LONG ret = 0;
        MWCloseImage(g_channel_handle, osd, &ret);
    }

    if(notify){
        MWUnregisterNotify(g_channel_handle, notify);
        notify = 0;
    }
    if(capture_start){
        MWStopVideoCapture(g_channel_handle);
    }
    if(notify_event!= 0){
        MWCloseEvent(notify_event);
        notify_event = 0;
    }

    if(capture_event!= 0){
        MWCloseEvent(capture_event);
        capture_event = 0;
    }
}


void list_device()
{
    int capture_num;
    MWCAP_CHANNEL_INFO mci;
    MWRefreshDevice();
    capture_num = MWGetChannelCount();
    for (int i = 0; i < capture_num; i++) {
        MWGetChannelInfoByIndex(i, &mci);
        if(NULL == strstr(mci.szProductName, "Pro Capture")){
            continue;
        }
        sprintf(g_capture_name[g_capture_num], "%d %s", i, mci.szProductName);
        g_capture_index[g_capture_num] = i;
        g_capture_num++;
        if(g_capture_num >= MAX_CAPTURE){
            printf("too many capture, just list %d\n", MAX_CAPTURE);
            break;
        }
    }
}



void imgui_clieck_device(int index, bool is_choose)
{
    MWCAP_CHANNEL_INFO channel_info;
    static HANDLE video_handle = NULL;
    void *status;
    static pthread_t video_capture_tid = 0;
    g_video_capture = 0;
    if(video_capture_tid){
        pthread_join(video_capture_tid, &status);
        video_capture_tid = 0;
    }
    if(g_channel_handle){
        MWCloseChannel(g_channel_handle);
        g_channel_handle = NULL;
    }
    if(0 == is_choose){
        return;
    }
    char path[256] = {0};
    if(MW_SUCCEEDED != MWGetDevicePath(index, path)){
        printf("%s get device path fail\n", g_capture_name[index]);
        return;
    }
    
    g_channel_handle = MWOpenChannelByPath(path);
    if(NULL == g_channel_handle){
        printf("%s open channel fail\n", g_capture_name[index]);
        return;
    }
    g_video_capture = 1;
    pthread_create(&video_capture_tid, NULL, video_capture, NULL);
}

void imgui_init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsClassic();

}


void imgui_choose()
{
    int i;
    bool refresh = 0;
    static bool choose[MAX_CAPTURE] = {0};
    static bool is_first = 1;
    if(is_first){
        imgui_clieck_device(g_capture_index[0], 1);
        memset(choose, 0, sizeof(choose));
        choose[0] = 1;
        is_first = 0;
    }
    ImGui_ImplGlfwGL3_NewFrame();
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Devices")) {
            for (i = 0; i < g_capture_num; i++) {
                if (ImGui::MenuItem(g_capture_name[i], NULL, &choose[i])) {
                    imgui_clieck_device(g_capture_index[i], choose[i]);
                    if(choose[i]){
                        memset(choose, 0, sizeof(choose));
                        choose[i] = 1;
                    }
                }
            }
            /*if(ImGui::MenuItem("refresh device", NULL, refresh)){
                memset(choose, 0, sizeof(choose));
                imgui_clieck_device(0, 0);
                list_device();
            }*/
            ImGui::EndMenu();
        }
       if (ImGui::BeginMenu("OSD")) {
            if(ImGui::MenuItem("text", NULL, &g_text)){
                printf("text %d\n",g_text);
            }
            if(ImGui::MenuItem("border", NULL, &g_border)){
                printf("border %d\n",g_border);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void imgui_render()
{
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    
}

void imgui_deinit()
{
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
}


static void glfw_error_callback(int error, const char* description)
{
    printf("error callback %d: %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}


void video_render()
{
    int play_index;
    int dis_w = 960;
    int dis_h = 540;
    GLFWwindow* window = NULL;
    MWOpenGLRender video_render;
    GLFWvidmode* mode;
    mw_render_init_t rinit;
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        printf("glfwInit fail\n");
        return;
    }
    mode = (GLFWvidmode*)glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode){
        dis_w = mode->width / 2;
        dis_h = mode->height/2;
    }

    window = glfwCreateWindow(dis_w, dis_h, "OSDPreview", NULL, NULL);
    if(NULL == window){
        printf("create window fail\n");
        return;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glewInit();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    imgui_init(window);

    rinit.buffers_num = 0;
    rinit.height = CAPTURE_HEIGHT;
    rinit.width = CAPTURE_WIDTH;
    rinit.render_fourcc = RENDER_FOURCC;
    video_render.open(&rinit);
    glClearColor(0.2, 0.3, 0.3, 1.0);
    while (!glfwWindowShouldClose(window)) {
        mw_render_ctrl_t ctrl;
        glClear(GL_COLOR_BUFFER_BIT);
        imgui_choose();
        glfwGetFramebufferSize(window, &dis_w, &dis_h);
        ctrl.display_h = dis_h;
        ctrl.display_w = dis_w;
        ctrl.threshold = 40;
        ctrl.val_ctrl = 40;
        ctrl.hdr_on = 0;
        if(g_video_capture){
            play_index = g_vfw_index - 1;
            if(play_index < 0){
                play_index = VIDEO_FRAME_NUM - 1;
            }
            if(video_render.render(g_video_frame[play_index], &ctrl, -1)){
                break;
            }
        }else{
            glClearColor(0.2, 0.3, 0.3, 1.0);
        }
        imgui_render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    video_render.close();
    imgui_clieck_device(0, 0);
    imgui_deinit();
    if(window){
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}


int main()
{
    int i;
    
    print_version();
    if(!MWCaptureInitInstance()){
        printf("InitilizeFailed\n");
        return -1;
    }
    g_min_stride = FOURCC_CalcMinStride(CAPTURE_FOURCC, CAPTURE_WIDTH, 4);
    g_image_size = FOURCC_CalcImageSize(CAPTURE_FOURCC, CAPTURE_WIDTH, CAPTURE_HEIGHT, g_min_stride);
    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        g_video_frame[i] = (unsigned char*)malloc(g_image_size);
        if(NULL == g_video_frame[i]){
            printf("video cache malloc fail");
            goto end_and_free;
        }
    }
    image = (unsigned char *)malloc(CAPTURE_WIDTH * CAPTURE_HEIGHT * 4);
    if(NULL == image){
        printf("image malloc fail");
        goto end_and_free;
    }
    g_running = 1;
    list_device();
    video_render();
    g_running = 0;
end_and_free:
    MWCaptureExitInstance();

    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        if(g_video_frame[i]){
            free(g_video_frame[i]);
        }
    }
    if(image){
        free(image);
    }
    printf("AVCapture stoped\n");
    return 0;
}
