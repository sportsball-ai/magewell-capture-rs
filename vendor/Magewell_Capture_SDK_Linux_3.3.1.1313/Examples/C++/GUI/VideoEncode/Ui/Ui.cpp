#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "mw_venc.h"
#include "OpenGLRender.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "VideoCapture.h"
#include "VideoEncode.h"
#include "Ui.h"
#define CHOSE_DEVICE 0
#define GET_CAPTURE_SUPPORT 1
#define SET_CAPTURE_PARM 2
#define GET_ENCODE_SUPPORT 3
#define SET_ENCODE_PARM 4
#define ENCODING 5



typedef struct encode_param{
    int flag;
    int platfrom_index;
    int amd_mem_index;
    int code_index;
    int usage_index;
    int rate_control_index;
    int bitrate;  
    int max_bitrate;
    int targe_bitrate;
    int qpi;
    int qpp;
    int qpb;
    int slice_num;
    int gop_size;
    int profile_index;
}encode_param_t;

MWOpenGLRender g_video_render;
int32_t g_device_index = -1;
int32_t g_ui_status = CHOSE_DEVICE;

encode_param_t g_save_param_select = {0};
encode_param_t g_param_select = {0};
char g_out_file_name[256]={0};
char g_capture_status[64];
char g_encode_status[128];

char* g_string_targetusage[] = {"Best Speed","Balanced","Best Quality"};
mw_venc_targetusage_t g_venc_targetusage[] = {MW_VENC_TARGETUSAGE_BEST_QUALITY, MW_VENC_TARGETUSAGE_BALANCED, MW_VENC_TARGETUSAGE_BEST_SPEED};
char* g_string_rate_control[] = {"CBR","VBR","CQP"};
mw_venc_rate_control_mode_t g_venc_crate_control_mode[] = {MW_VENC_RATECONTROL_CBR, MW_VENC_RATECONTROL_VBR, MW_VENC_RATECONTROL_CQP};
char* g_string_profile[] = {"H264 Baseline","H264 Main","H264 High", "H265 Main"};
mw_venc_profile_t g_venc_profile[] = {MW_VENC_PROFILE_H264_BASELINE, MW_VENC_PROFILE_H264_MAIN, MW_VENC_PROFILE_H264_HIGH, MW_VENC_PROFILE_H265_MAIN};

mw_render_init_t g_rinit;
static int32_t g_render_frames = 0;
float get_now_render_fps()
{
    static float fps = 0.0;
    static int prev_time = 0;
    int now_time;
    static int prev_frame_num = 0;
    int step = 100;

    if(g_render_frames < prev_frame_num){
		fps = 0.0;
        prev_frame_num = g_render_frames;
    }
    if(g_render_frames < 50){
        step = 5;
    }
    if((g_render_frames - prev_frame_num) < step){
        return fps;
    }
    now_time = GetTickCount();
    if(prev_time == 0){
        prev_time = now_time;
        prev_frame_num = g_render_frames;
        return fps;
    }
    if(0 == (now_time - prev_time)){
        prev_frame_num = g_render_frames;
        return fps;
    }
    fps = (g_render_frames - prev_frame_num) *1000.0 / (now_time - prev_time);
    prev_time = now_time;
    prev_frame_num = g_render_frames;
    return fps;
}

void fourcc_format(int32_t capture_fourcc, char *p_char, uint32_t *p_render, mw_venc_fourcc_t *p_venc)
{
    if(CAPTURE_FOURCC_YUYV == capture_fourcc){
        if(p_char){
            memcpy(p_char, "YUY2", 5);
        }
        if(p_render){
            *p_render = RENDER_YUY2;;
        }
        if(p_venc){
            *p_venc = MW_VENC_FOURCC_YUY2;
        }
    }else{
        if(p_char){
            memcpy(p_char, "NV12", 5);
        }
        if(p_render){
            *p_render = RENDER_NV12;;
        }
        if(p_venc){
            *p_venc = MW_VENC_FOURCC_NV12;
        }
    }
}

void first_set_encode_param()
{
    if(g_param_select.flag){
        return;
    }
    memset(&g_param_select, 0, sizeof(g_param_select));
    g_param_select.bitrate = 4096;
    g_param_select.max_bitrate = 4096;
    g_param_select.targe_bitrate = 4096;
    g_param_select.qpi = 26;
    g_param_select.qpp = 26;
    g_param_select.qpb = 26;
    g_param_select.slice_num = 1;
    g_param_select.gop_size = 60;
    g_param_select.flag = 1;
    memcpy(&g_save_param_select, &g_param_select, sizeof(g_param_select));
}
void generate_file_name(char *p_type)
{
    time_t timep;   
    struct tm *p; 
    time(&timep);   
    p = localtime(&timep);
    sprintf(g_out_file_name, "%d-%d-%d_%d-%d-%d-%s", 1900 + p->tm_year,
        1 + p->tm_mon,  p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, p_type);
    printf("out file name is: %s\n", g_out_file_name);
}

int32_t click_start_encode()
{
    int32_t fourcc;
    mw_venc_param_t enc_param;
    mw_venc_get_default_param(&enc_param);
    if(SET_ENCODE_PARM != g_ui_status){
        first_set_encode_param();
    }
    get_capture_param(&enc_param.width, &enc_param.height, &fourcc, &enc_param.fps.den, &enc_param.fps.num);;
    fourcc_format(fourcc, NULL, NULL, &enc_param.fourcc);
    if(0 == g_save_param_select.code_index){
        generate_file_name("h264");
        enc_param.code_type = MW_VENC_CODE_TYPE_H264;
        enc_param.profile = g_venc_profile[g_save_param_select.profile_index];
    }else{
        generate_file_name("h265");
        enc_param.code_type = MW_VENC_CODE_TYPE_H265;
        enc_param.profile = g_venc_profile[g_save_param_select.profile_index + 3];
    }
    enc_param.targetusage = g_venc_targetusage[g_save_param_select.usage_index];
    enc_param.rate_control.mode = g_venc_crate_control_mode[g_save_param_select.rate_control_index];
    if(0 == g_save_param_select.rate_control_index){
        enc_param.rate_control.target_bitrate = g_save_param_select.bitrate;
    }else if(1 == g_save_param_select.rate_control_index){
        enc_param.rate_control.target_bitrate = g_save_param_select.bitrate;
        enc_param.rate_control.max_bitrate = g_save_param_select.max_bitrate;
    }else{
        enc_param.rate_control.qpi = g_save_param_select.qpi;
        enc_param.rate_control.qpp = g_save_param_select.qpp;
        enc_param.rate_control.qpb = g_save_param_select.qpb;
    }
    enc_param.slice_num = g_save_param_select.slice_num;
    enc_param.gop_pic_size = g_save_param_select.gop_size;
    enc_param.gop_ref_size = 1;
    enc_param.level = MW_VENC_LEVEL_5_2;
    return start_encode(g_save_param_select.platfrom_index, &enc_param, g_out_file_name);
}
int click_stop_encode()
{
    g_out_file_name[0] = 0;
    return stop_encode();
}
void imgui_init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsClassic();
}

static char *g_p_platfrom[MAX_PLATFROM_NUM];
static int g_platfrom_num = 0;
void imgui_draw_video_capture()
{
    struct ImVec2 vec = ImVec2(0,0);
    static int resolution_select = 0;
    static int fps_select = 0;
    static int fourcc_select = 0;
    static int save_resolution_select = -1;
    static int save_fps_select = -1;
    static int save_fourcc_select = -1;
    static char *p_resolution[MAX_RESOLUTION_NUM];
    static char *p_fps[MAX_FPS_NUM];
    static char *p_fourcc[MAX_FOURCC_NUM];
    static int32_t resolution_num;// = get_support_resolution(g_device_index, &p_resolution);
    static int32_t fps_num;// = get_support_fps(g_device_index, &p_fps);
    static int32_t fourcc_num;// = get_support_fourcc(g_device_index, &p_fourcc);
    if(g_ui_status == GET_CAPTURE_SUPPORT){
        resolution_select = 0;
        fps_select = 0;
        fourcc_select = 0;
        save_resolution_select = -1;
        save_fps_select = -1;
        save_fourcc_select = -1;
        resolution_num = get_support_resolution(g_device_index, p_resolution);
        fps_num = get_support_fps(g_device_index, p_fps);
        fourcc_num = get_support_fourcc(g_device_index, p_fourcc);
        g_ui_status = SET_CAPTURE_PARM;
    }
    ImGui::Combo("resolution", &resolution_select, p_resolution, resolution_num);
    ImGui::Combo("fps", &fps_select, p_fps, fps_num);
    ImGui::Combo("color", &fourcc_select, p_fourcc, fourcc_num);
    if(ImGui::Button("  OK  ",vec)){
        if((resolution_select != save_resolution_select) ||
          (fps_select != save_fps_select) ||
          (fourcc_select != save_fourcc_select)){
            //render_init rinit;
            mw_venc_fourcc venc_fourcc;
            int32_t width, height, fourcc, fps_d, fps_n;
            save_resolution_select = resolution_select;
            save_fps_select = fps_select;
            save_fourcc_select = fourcc_select;
            stop_capture();
            if(start_capture(g_device_index, resolution_select, fps_select, fourcc_select)){
                sprintf(g_capture_status, "capture status:fail");
            }else{
                sprintf(g_capture_status, "capture status:capturing");
                get_capture_param(&width, &height, &fourcc, &fps_d, &fps_n);
                g_video_render.close();
                g_rinit.height = height;
                g_rinit.width = width;
                fourcc_format(fourcc, NULL, &g_rinit.render_fourcc, &venc_fourcc);
                g_video_render.open(&g_rinit);
                g_param_select.flag = 0;
                g_platfrom_num = get_support_platfrom(g_p_platfrom, width, height, venc_fourcc);
                g_ui_status = GET_ENCODE_SUPPORT;
            }

        }
    }
    ImGui::SameLine();
    ImGui::Text("               ");
    ImGui::SameLine();
    if(ImGui::Button("Cancel",vec)){
        resolution_select = (save_resolution_select < 0) ? 0 : save_resolution_select;
        fps_select = (save_fps_select < 0) ? 0 : save_fps_select;
        fourcc_select = (save_fourcc_select < 0) ? 0 : save_fourcc_select;
    }
}

void imgui_draw_encode()
{
    encode_param_t save_param_selectw;
//    encode_param_t param_select;
    static char fourcc[8];
    static int32_t width;
    static int32_t height;
    static int32_t fps_d, fps_n;
    if((GET_ENCODE_SUPPORT == g_ui_status) || (0 == g_param_select.flag)){
        int32_t fourcc_type;
        first_set_encode_param();
        get_capture_param(&width, &height, &fourcc_type, &fps_d, &fps_n);
        fourcc_format(fourcc_type, fourcc, NULL, NULL);
        g_ui_status = SET_ENCODE_PARM;
    }
    if(g_platfrom_num <= 0){
        return;
    }
    ImGui::Combo("platform", &g_param_select.platfrom_index, g_p_platfrom, g_platfrom_num);

    ImGui::Separator();

    // encode type
    ImGui::RadioButton("H264", &g_param_select.code_index, 0);
    ImGui::SameLine();
    ImGui::RadioButton("H265", &g_param_select.code_index, 1);

    ImGui::Separator();

    // fourcc
    // same with capture
    ImGui::LabelText("fourcc", "%s", fourcc);

    ImGui::Separator();

    // target usage
    ImGui::Combo("target uasge", &g_param_select.usage_index, g_string_targetusage, 3);

    ImGui::Separator();

        // rate control
        // cbr vbr cqp
    ImGui::Combo("rate control", &g_param_select.rate_control_index, g_string_rate_control, 3);
    if (g_param_select.rate_control_index == 0) {
        // cbr: constant bit rate
        ImGui::InputInt("target bitrate", &g_param_select.bitrate);
        if(g_param_select.bitrate<100||g_param_select.bitrate>100000){
            g_param_select.bitrate = 4096;
        }
    }
    else if (g_param_select.rate_control_index == 1) {
        // vbr: variable bit rate
        ImGui::InputInt("target bitrate", &g_param_select.targe_bitrate);
        if(g_param_select.targe_bitrate<100||g_param_select.targe_bitrate>100000){
            g_param_select.targe_bitrate = 4096;
        }
        ImGui::InputInt("max bitrate", &g_param_select.max_bitrate);
        if(g_param_select.max_bitrate<100||g_param_select.max_bitrate>100000)
            g_param_select.max_bitrate = 4096;
        if(g_param_select.max_bitrate<g_param_select.targe_bitrate)
            g_param_select.max_bitrate = g_param_select.targe_bitrate;
    }
    else {
        // cqp: const q p
        ImGui::InputInt("I", &g_param_select.qpi);
        if((g_param_select.qpi < 0) || (g_param_select.qpi > 51))
            g_param_select.qpi = 26;
        ImGui::InputInt("P", &g_param_select.qpp);
        if((g_param_select.qpi < 0) || (g_param_select.qpi > 51))
            g_param_select.qpi = 26;
        ImGui::InputInt("B", &g_param_select.qpb);
        if((g_param_select.qpi < 0) || (g_param_select.qpi > 51))
            g_param_select.qpi = 26;
    }

    ImGui::Separator();

    // width
    // same with capture
    ImGui::LabelText("width", "%d", width);

    // height
    // same with capture
    ImGui::LabelText("height", "%d", height);

    ImGui::Separator();

    // fps
    // same with capture
    ImGui::LabelText("fps", "%.2f %d/%d", fps_n*1.0/fps_d, fps_n, fps_d);

    ImGui::Separator();

    // slice num
    ImGui::InputInt("slice num", &g_param_select.slice_num);
    if((g_param_select.slice_num <= 0) || (g_param_select.slice_num > 32)){
        g_param_select.slice_num = 1;
    }

    ImGui::Separator();

    // gop size
    ImGui::InputInt("gop size", &g_param_select.gop_size);
    if((g_param_select.gop_size <= 0) || (g_param_select.gop_size > 1000)){
        g_param_select.gop_size = 30;
    }
    ImGui::Separator();

    // profile
    if (g_param_select.code_index == 0) {
        ImGui::Combo("H264 profile", &g_param_select.profile_index, g_string_profile, 3);
    }
    else if (g_param_select.code_index == 1) {
        ImGui::Combo("H265 profile", &g_param_select.profile_index, &g_string_profile[3], 1);
    }


    ImGui::Separator();

    if(ImGui::Button("  OK  ")){
        memcpy(&g_save_param_select, &g_param_select, sizeof(g_param_select));
    }
    ImGui::SameLine();
    ImGui::Text("                         ");
    ImGui::SameLine();
    if(ImGui::Button("Cancel")){
        memcpy(&g_param_select, &g_save_param_select, sizeof(g_param_select));
    }
}

void get_text(char *p_text)
{
    int32_t gb,mb,kb,b;
    long long size = get_now_encode_size();
    gb = size/1024/1024/1024;
    size = size%(1024*1024*1024);
    mb = size/1024/1024;
    size = size%(1024*1024);
    kb = size/1024;
    b = size%1024;
    sprintf(p_text, "%s\n"
        "capture fps:%f capture frams:%d\n"
        "render fps:%f render frams:%d\n"
        "%s\n"
        "encode fps:%f encode frames:%d encode size:%dGB %dMB %dKB %dB\n"
        "outputfile:%s", 
        g_capture_status, 
        get_now_capture_fps(), get_now_capture_frames(), 
        get_now_render_fps(), g_render_frames,
        g_encode_status, 
        get_now_encode_fps(), get_now_encode_frames(),gb,mb,kb,b,
        g_out_file_name);
}

void imgui_draw_text()
{
    char text[1024];
    bool m_b_open = true;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos;
    ImVec2 size;
    get_text(text);
    size.x = 600;
    size.y = 90;
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
void imgui_draw()
{
    int i;
    bool refresh = 0;
    static bool choose[MAX_CAPTURE] = {0};
    char *p_device_name[MAX_CAPTURE];
    int32_t capture_num = get_device_list(p_device_name);
    ImGui_ImplGlfwGL3_NewFrame();
    if (ImGui::BeginMainMenuBar()) {
       if (ImGui::BeginMenu("file")) {
            if (ImGui::BeginMenu("video capture param", (g_ui_status >= GET_CAPTURE_SUPPORT) && (g_ui_status < ENCODING))) {
                imgui_draw_video_capture();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("encode param", (g_ui_status >= GET_ENCODE_SUPPORT) && (g_ui_status < ENCODING) && g_platfrom_num)) {
                imgui_draw_encode();
                ImGui::EndMenu();
            }
            if(ImGui::MenuItem("start encode", NULL, false, (g_ui_status >= GET_ENCODE_SUPPORT) && (g_ui_status < ENCODING) && g_platfrom_num)){
                if(click_start_encode()){
                    sprintf(g_encode_status, "This graphic adapter doesn't support current encode param,\nplease try with another param.");
                    g_out_file_name[0] = 0;
                }else{
                    sprintf(g_encode_status, "encode status:encoding");
                    g_ui_status = ENCODING;
                }
            }
            if(ImGui::MenuItem("stop encode", NULL, false, g_ui_status == ENCODING)){
                click_stop_encode();
                sprintf(g_encode_status, "encode status:encode stopped");
                g_ui_status = SET_ENCODE_PARM;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Devices")) {
            for (i = 0; i < capture_num; i++) {
                if (ImGui::MenuItem(p_device_name[i], NULL, &choose[i], g_ui_status != ENCODING)) {
                    if(g_device_index != i){
                        stop_capture();
                        g_device_index = i;
                        sprintf(g_encode_status, "encode status:encode stopped");
                        sprintf(g_capture_status, "capture status:video capture stopped");
                        g_out_file_name[0] = 0;
                        g_ui_status = GET_CAPTURE_SUPPORT;
                    }
                    memset(choose, 0, sizeof(choose));
                    choose[i] = true;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
        imgui_draw_text();
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

void ui_opengl_render()
{
    int buffers_index = 0;
    int dis_w = 960;
    int dis_h = 540;
    GLFWwindow* window = NULL;
    GLFWvidmode* mode;
    //render_init rinit;
    uint8_t *p_render_data;
    sprintf(g_capture_status, "capture status:video capture stopped");
    sprintf(g_encode_status, "encode status:encode stopped");
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

    window = glfwCreateWindow(dis_w, dis_h, "VideoEncode", NULL, NULL);
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

    g_rinit.height = 480;
    g_rinit.width = 720;
    g_rinit.render_fourcc = RENDER_NV12;
    g_video_render.open(&g_rinit);
    glClearColor(0.2, 0.3, 0.3, 1.0);
    while (!glfwWindowShouldClose(window)) {
        int sleep_count = 0;
        mw_render_ctrl_t ctrl;
        glClear(GL_COLOR_BUFFER_BIT);
        imgui_draw();
        glfwGetFramebufferSize(window, &dis_w, &dis_h);
        ctrl.display_h = dis_h;
        ctrl.display_w = dis_w;
        ctrl.threshold = 40;
        ctrl.val_ctrl = 40;
        ctrl.hdr_on = 0;
        p_render_data = NULL;
        while(get_render_data(&p_render_data) > 0){
            usleep(10);
            sleep_count++;
            if(sleep_count > 100000){
                break;
            }
        }
        if(NULL == p_render_data){
            glClearColor(0.2, 0.3, 0.3, 1.0);
        }else{
            g_video_render.render(p_render_data, &ctrl);
            g_render_frames++;
        }
        imgui_render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    g_video_render.close();
    imgui_deinit();
    if(window){
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

