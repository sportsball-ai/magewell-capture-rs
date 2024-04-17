#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>

#include "ProductVer.h"
#include "LibMWCapture/MWCapture.h"
#include "MWFOURCC.h"

#include "OpenGLRender/OpenGLRender.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "AlsaPlayer/AlsaPlayerDevice.h"

#define MAX_CAPTURE 16
#define VIDEO_FRAME_NUM 3


#define CAPTURE_CHANNEL 2
#define CAPTURE_BIT_PER_SAMPLE 16
#define CAPTURE_SAMPLE_PER_FRAME 192



static int CAPTURE_SAMPLE_RATE = 48000;
static int CAPTURE_FRAME_RATE = 25;
static int CAPTURE_WIDTH = 1920;
static int CAPTURE_HEIGHT = 1080;
static unsigned int CAPTURE_FOURCC = MWFOURCC_YUY2;
static unsigned int RENDER_FOURCC = RENDER_YUY2;

static int g_image_size = 0;
static int g_min_stride = 0;

static unsigned char *g_video_frame[VIDEO_FRAME_NUM];
static int g_vfc_index = 1;

static unsigned char *g_audio_cache;
static int g_audio_cache_size = (CAPTURE_SAMPLE_PER_FRAME * CAPTURE_CHANNEL * CAPTURE_BIT_PER_SAMPLE / 2 * 500);
static long long g_afcw_byte = 0;
static long long g_afcr_byte = 0;


static int g_video_capture = 0;
static int g_audio_capture = 0;
static int g_running = 0;


static int g_capture_num = 0;
static char g_capture_name[MAX_CAPTURE][80];

static int g_audio_capture_frames = 0;
static int g_audio_play_frames = 0;
static int g_video_capture_frames = 0;
static int g_video_render_frames = 0;


void print_version()
{
    BYTE maj, min;
    WORD build;
    MWGetVersion(&maj, &min, &build);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - AVCapture2\n", maj, min, build);
    printf("Usage:\n./AVCapture [-fourcc nv12] [-width 1920] [-height 1080] [-sample_rate 48000] [-frame_rate 25]\n");
}


void *audio_player(void *)
{
    int frame_len = 2 * CAPTURE_CHANNEL * CAPTURE_SAMPLE_PER_FRAME;
    int samples;
    int offset;
    ALSAPlayerDevice audio_player;
    audio_player.create_device("default");
    audio_player.SetFormat(CAPTURE_SAMPLE_RATE, CAPTURE_CHANNEL, CAPTURE_SAMPLE_PER_FRAME);
    audio_player.SetVolume(99,0,100);
    while(g_running&g_audio_capture){
        if((g_afcw_byte - g_afcr_byte) < frame_len){
            usleep(10000);
            continue;
        }
        if((g_afcw_byte - g_afcr_byte) > (g_audio_cache_size / 2)){
            g_afcr_byte = g_afcw_byte - frame_len;
        }
        g_audio_play_frames++;
        offset = g_afcr_byte % g_audio_cache_size;
        audio_player.write_data(g_audio_cache + offset, CAPTURE_SAMPLE_PER_FRAME);
        g_afcr_byte += frame_len;
    }
    audio_player.destory_device();
}

void list_device()
{
    
    MWCAP_CHANNEL_INFO mci;
    MWRefreshDevice();
    g_capture_num = MWGetChannelCount();
    for (int i = 0; i < g_capture_num; i++) {
        MWGetChannelInfoByIndex(i, &mci);    
        sprintf(g_capture_name[i], "%d %s", i, mci.szProductName);
    }
}


void video_frame_callback(BYTE *frame_buf, long buf_len, void* parm)
{
    memcpy(g_video_frame[g_vfc_index], frame_buf, buf_len);
    g_vfc_index++;
    g_video_capture_frames++;
    if(g_vfc_index >= 3){
        g_vfc_index = 0;
    }
}

void audio_frame_callback(const BYTE * frame_buf, int buf_len, uint64_t time_stamp, void* parm)
{
    int offset = g_afcw_byte % g_audio_cache_size;
    int left_len = g_audio_cache_size - offset;
    g_audio_capture_frames++;
    if(buf_len > left_len){
        memcpy(g_audio_cache + offset, frame_buf, left_len);
        memcpy(g_audio_cache, frame_buf + left_len, buf_len - left_len);
    }else{
        memcpy(g_audio_cache + offset, frame_buf, buf_len);
    }
    g_afcw_byte += buf_len;
}



void imgui_clieck_device(int index, bool is_choose)
{
    static HCHANNEL channel_handle = NULL;
    static HANDLE video_handle = NULL;
    static HANDLE audio_handle = NULL;
    void *status;
    static pthread_t audio_player_tid = 0;
    if(video_handle){
        MWDestoryVideoCapture(video_handle);
        video_handle = NULL;
        g_video_capture = 0;
    }
    if(audio_handle){
        MWDestoryAudioCapture(audio_handle);
        audio_handle = NULL;
        g_audio_capture = 0;
    }
    if(channel_handle){
        MWCloseChannel(channel_handle);
        channel_handle = NULL;
    }
    if(audio_player_tid){
        pthread_join(audio_player_tid, &status);
        audio_player_tid = 0;
        g_afcr_byte = g_afcw_byte = 0;
    }
    g_audio_capture_frames = 0;
    g_video_capture_frames = 0;
    g_audio_play_frames = 0;
    g_video_render_frames = 0;
    if(0 == is_choose){
        return;
    }
    char path[256] = {0};
    if(MW_SUCCEEDED != MWGetDevicePath(index, path)){
        printf("%s get device path fail\n", g_capture_name[index]);
        return;
    }
    
    channel_handle = MWOpenChannelByPath(path);
    if(NULL == channel_handle){
        printf("%s open channel fail\n", g_capture_name[index]);
        return;
    }

    audio_handle = MWCreateAudioCapture(channel_handle, MWCAP_AUDIO_CAPTURE_NODE_DEFAULT, 
        CAPTURE_SAMPLE_RATE, CAPTURE_BIT_PER_SAMPLE, CAPTURE_CHANNEL, audio_frame_callback, NULL);
    if (audio_handle){
        g_audio_capture = 1;
    }else{
        printf("%s capture audio fail\n", g_capture_name[index]);
    }
    pthread_create(&audio_player_tid, NULL, audio_player, NULL);
    video_handle = MWCreateVideoCapture(channel_handle, CAPTURE_WIDTH, CAPTURE_HEIGHT, 
        CAPTURE_FOURCC, 10000000/CAPTURE_FRAME_RATE, video_frame_callback, NULL);
    if (video_handle){
        g_video_capture = 1;
    }else{
        printf("%s capture video fail\n", g_capture_name[index]);
    }
}

void imgui_init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsClassic();

}


void imgui_choose_device()
{
    int i;
    bool refresh = 0;
    static bool choose[MAX_CAPTURE] = {0};
    static bool is_first = 1;
    if(is_first){
        imgui_clieck_device(0, 1);
        memset(choose, 0, sizeof(choose));
        choose[0] = 1;
        is_first = 0;
    }
    ImGui_ImplGlfwGL3_NewFrame();
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Devices")) {
            for (i = 0; i < g_capture_num; i++) {
                if (ImGui::MenuItem(g_capture_name[i], NULL, &choose[i])) {
                    imgui_clieck_device(i, choose[i]);
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
        ImGui::EndMainMenuBar();
    }
}

int GetTickCount()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}

void get_fps_text(char *p_text)
{
    static int prev_time = 0;
    static int count = 0;
    static int prev_audio_capture_frames = 0;
    static int audio_play_frames = 0;
    static int prev_video_capture_frames = 0;
    static int prev_video_render_frames = 0;

    static float audio_capture_fps = 0.0;
    static float audio_play_fps = 0.0;
    static float video_capture_fps = 0.0;
    static float video_render_fps = 0.0;

    if(count == 0){
        prev_time = GetTickCount();
    }else if(count < 100){
        int time=GetTickCount()-prev_time;
        if(time){
            audio_capture_fps = (g_audio_capture_frames - prev_audio_capture_frames)*1000.0 / time;
            audio_play_fps = (g_audio_play_frames - audio_play_frames)*1000.0 / time;
            video_capture_fps = (g_video_capture_frames - prev_video_capture_frames)*1000.0 / time;
            video_render_fps = (g_video_render_frames - prev_video_render_frames)*1000.0 / time;
        }
    }else if((count % 100) == 0){
        int now_time = GetTickCount();
        if(now_time != prev_time){
            int time = now_time - prev_time;
            audio_capture_fps = (g_audio_capture_frames - prev_audio_capture_frames)*1000.0 / time;
            audio_play_fps = (g_audio_play_frames - audio_play_frames)*1000.0 / time;
            video_capture_fps = (g_video_capture_frames - prev_video_capture_frames)*1000.0 / time;
            video_render_fps = (g_video_render_frames - prev_video_render_frames)*1000.0 / time;
            prev_audio_capture_frames = g_audio_capture_frames;
            audio_play_frames = g_audio_play_frames;
            prev_video_capture_frames = g_video_capture_frames;
            prev_video_render_frames= g_video_render_frames;
            prev_time = now_time;
        }
    }
    if(g_audio_capture_frames == 0){
        audio_capture_fps = 0.0;
    }
    if(g_audio_play_frames == 0){
        audio_play_fps = 0.0;
    }

    if(g_video_capture_frames == 0){
        video_capture_fps = 0.0;
    }
    if(g_video_render_frames == 0){
        video_render_fps = 0.0;
    }

    count++;
    sprintf(p_text,
        "capture fps(v|a):%f|%f capture frams(v|a):%d|%d\n"
        "render fps(v|a):%f|%f render frams(v|a):%d|%d\n", 
        video_capture_fps, audio_capture_fps, g_video_capture_frames, g_audio_capture_frames,
        video_render_fps, audio_play_fps, g_video_render_frames,g_audio_play_frames);
}

void imgui_draw_fps()
{
    char text[1024];
    bool m_b_open = true;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos;
    ImVec2 size;
    get_fps_text(text);
    size.x = 500;
    size.y = 40;
    ImGuiWindowFlags m_imgui_flags = ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoTitleBar | 
                        ImGuiWindowFlags_NoResize  | 
                        ImGuiWindowFlags_NoScrollbar | 
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoNav;
        pos.x = io.DisplaySize.x - size.x - 10;
        pos.y = io.DisplaySize.y - size.y - 10;
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.5);
        ImGui::Begin("status", &m_b_open, m_imgui_flags);
        ImGui::Text("%s", text);
        ImGui::End();

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
    int play_index = 0;
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

    window = glfwCreateWindow(dis_w, dis_h, "AVCapture2", NULL, NULL);
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

    rinit.height = CAPTURE_HEIGHT;
    rinit.width = CAPTURE_WIDTH;
    rinit.render_fourcc = RENDER_FOURCC;
    video_render.open(&rinit);
    glClearColor(0.2, 0.3, 0.3, 1.0);
    while (!glfwWindowShouldClose(window)) {
        int sleep_count=0;
        mw_render_ctrl_t ctrl;
        glClear(GL_COLOR_BUFFER_BIT);
        imgui_choose_device();
        imgui_draw_fps();
        glfwGetFramebufferSize(window, &dis_w, &dis_h);
        ctrl.display_h = dis_h;
        ctrl.display_w = dis_w;
        ctrl.threshold = 40;
        ctrl.val_ctrl = 40;
        ctrl.hdr_on = 0;
        if(g_video_capture){
            while(g_video_capture && (play_index == g_vfc_index)){
                sleep_count++;
                if(sleep_count > 100000){
                    play_index = g_vfc_index;
                    printf("one second have not data\n");
                    break;
                }
                usleep(10);
                continue;                
            }
            g_video_render_frames++;
            if(video_render.render(g_video_frame[play_index], &ctrl)){
                break;
            }
            play_index++;
            if(play_index >= VIDEO_FRAME_NUM){
                play_index = 0;
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


void set_fourcc()
{
    if(CAPTURE_FOURCC == MWFOURCC_NV12){
        printf("set fourcc to nv12\n");
    }else if(CAPTURE_FOURCC == MWFOURCC_YV12){
        printf("set fourcc to yv12\n");
        RENDER_FOURCC = RENDER_YV12;
    }else if(CAPTURE_FOURCC == MWFOURCC_I420){
        printf("set fourcc to i420\n");
        RENDER_FOURCC = RENDER_I420;
    }else if(CAPTURE_FOURCC == MWFOURCC_NV21){
        printf("set fourcc to nv21\n");
        RENDER_FOURCC = RENDER_NV21;
    }else if(CAPTURE_FOURCC == MWFOURCC_YUY2){
        printf("set fourcc to yuy2\n");
        RENDER_FOURCC = RENDER_YUY2;
    }else if(CAPTURE_FOURCC == MWFOURCC_RGB24){
        printf("set fourcc to rgb24\n");
        RENDER_FOURCC = RENDER_RGB24;
    }else if(CAPTURE_FOURCC == MWFOURCC_BGR24){
        printf("set fourcc to bgr24\n");
        RENDER_FOURCC = RENDER_BGR24;
    }else if(CAPTURE_FOURCC == MWFOURCC_RGBA){
        printf("set fourcc to rgba\n");
        RENDER_FOURCC = RENDER_RGBA;
    }else if(CAPTURE_FOURCC == MWFOURCC_BGRA){
        printf("set fourcc to bgra\n");
        RENDER_FOURCC = RENDER_BGRA;
    }else if(CAPTURE_FOURCC == MWFOURCC_ARGB){
        printf("set fourcc to argb\n");
        RENDER_FOURCC = RENDER_ARGB;
    }else if(CAPTURE_FOURCC == MWFOURCC_ABGR){
        printf("set fourcc to abgr\n");
        RENDER_FOURCC = RENDER_ABGR;
    }else{
        printf("not support fourcc, set to default nv12\n");
        CAPTURE_FOURCC == MWFOURCC_NV12;
    }
}

void parse_cmd(int argc, char* argv[])
{
    for(int i = 1; i < argc-1; i += 2){
        if(!memcmp(argv[i],"-width", strlen("-width"))){
            CAPTURE_WIDTH = atoi(argv[i+1]);
            printf("set width to %d\n",CAPTURE_WIDTH);
        }else if(!memcmp(argv[i],"-height", strlen("-height"))){
            CAPTURE_HEIGHT = atoi(argv[i+1]);
            printf("set height to %d\n",CAPTURE_HEIGHT);
        }else if(!memcmp(argv[i],"-fourcc", strlen("-fourcc"))){
            if(!memcmp(argv[i+1],"nv12", strlen("nv12"))){
                CAPTURE_FOURCC = MWFOURCC_NV12;
            }else if(!memcmp(argv[i+1],"yv12", strlen("yv12"))){
                CAPTURE_FOURCC = MWFOURCC_YV12;
            }else if(!memcmp(argv[i+1],"nv21", strlen("nv21"))){
                CAPTURE_FOURCC = MWFOURCC_NV21;
            }else if(!memcmp(argv[i+1],"i420", strlen("i420"))){
                CAPTURE_FOURCC = MWFOURCC_I420;
            }else if(!memcmp(argv[i+1],"yuy2", strlen("yuy2"))){
                CAPTURE_FOURCC = MWFOURCC_YUY2;
            }else if(!memcmp(argv[i+1],"rgb", strlen("rgb"))){
                CAPTURE_FOURCC = MWFOURCC_RGB24;
            }else if(!memcmp(argv[i+1],"bgr", strlen("bgr"))){
                CAPTURE_FOURCC = MWFOURCC_BGR24;
            }else if(!memcmp(argv[i+1],"argb", strlen("argb"))){
                CAPTURE_FOURCC = MWFOURCC_ARGB;
            }else if(!memcmp(argv[i+1],"abgr", strlen("abgr"))){
                CAPTURE_FOURCC = MWFOURCC_ABGR;
            }else if(!memcmp(argv[i+1],"rgba", strlen("rgba"))){
                CAPTURE_FOURCC = MWFOURCC_RGBA;
            }else if(!memcmp(argv[i+1],"bgra", strlen("bgra"))){
                CAPTURE_FOURCC = MWFOURCC_BGRA;
            }
            set_fourcc();
        }else if(!memcmp(argv[i],"-sample_rate", strlen("-sample_rate"))){
            CAPTURE_SAMPLE_RATE = atoi(argv[i+1]);
            printf("set sample rate to %d\n",CAPTURE_SAMPLE_RATE);
        }else if(!memcmp(argv[i],"-frame_rate", strlen("-frame_rate"))){
            CAPTURE_FRAME_RATE = atoi(argv[i+1]);
            printf("set frame rate to %d\n",CAPTURE_FRAME_RATE);
        }
    }
}

int main(int argc, char* argv[])
{
    int i;
    print_version();
    parse_cmd(argc, argv);
    if(!MWCaptureInitInstance()){
        printf("InitilizeFailed\n");
        return -1;
    }
    g_audio_cache = (unsigned char*)malloc(g_audio_cache_size);
    if(NULL == g_audio_cache){
        printf("audio cache malloc fail");
        goto end_and_free;
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
    g_running = 1;
    list_device();
    video_render();
    g_running = 0;
end_and_free:
    MWCaptureExitInstance();
    if(g_audio_cache){
        free(g_audio_cache);
    }
    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        if(g_video_frame[i]){
            free(g_video_frame[i]);
        }
    }
    printf("AVCapture2 stoped\n");
    return 0;
}
