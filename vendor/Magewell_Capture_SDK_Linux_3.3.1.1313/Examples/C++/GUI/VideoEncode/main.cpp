#include<stdio.h>
#include"VideoEncode.h"
#include"Ui.h"
#include"VideoCapture.h"

int main()
{
    init_capture();
    encode_init();
    ui_opengl_render();
    encode_deinit();
    deinit_capture();
    return 0;
}

