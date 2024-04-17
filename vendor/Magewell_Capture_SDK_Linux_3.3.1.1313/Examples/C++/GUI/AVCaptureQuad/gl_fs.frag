uniform sampler2D qt_Texture0;
uniform int cx;
uniform int cy;
uniform vec3 csp_const;
uniform vec3 csp_coeff_0;
uniform vec3 csp_coeff_1;
uniform vec3 csp_coeff_2;
varying vec2 qt_TexCoord0;

void main(void)
{
    vec3 rgb;
    vec3 yuv;

    highp float fcx=float(cx);
    highp float fcy=float(cy);
    int x=int(floor(fcx*qt_TexCoord0.s));
    int y=int(floor(fcy*qt_TexCoord0.t));
    int index=y*cx+x;
    int imod=x/2;
    int i=x-imod*2;
    if(i==0){
        highp float fxDD=fcx*qt_TexCoord0.s+1.0;
        fxDD=fxDD/fcx;
/*
        yuv.x=texture2D(qt_Texture0,qt_TexCoord0).r;
        yuv.y=texture2D(qt_Texture0,qt_TexCoord0).g-0.5;
        yuv.z=texture2D(qt_Texture0,vec2(fxDD,qt_TexCoord0.t)).g-0.5;
        rgb=mat3(1,1,1,\
                 0,-0.39465,2.03211,\
                 1.13983,-0.58060,0)*yuv;
*/
        yuv.x=texture2D(qt_Texture0,qt_TexCoord0).r;
        yuv.y=texture2D(qt_Texture0,qt_TexCoord0).g;
        yuv.z=texture2D(qt_Texture0,vec2(fxDD,qt_TexCoord0.t)).g;
        rgb.x = csp_coeff_0.x * yuv.x + csp_coeff_1.x * yuv.y + csp_coeff_2.x * yuv.z + csp_const.x;
        rgb.y = csp_coeff_0.y * yuv.x + csp_coeff_1.y * yuv.y + csp_coeff_2.y * yuv.z + csp_const.y;
        rgb.z = csp_coeff_0.z * yuv.x + csp_coeff_1.z * yuv.y + csp_coeff_2.z * yuv.z + csp_const.z;

       gl_FragData[0]=vec4(rgb,1);
    }
    else{
        highp float fxDD=fcx*qt_TexCoord0.s-1.0;
        fxDD=fxDD/fcx;
/*
        yuv.x=texture2D(qt_Texture0,qt_TexCoord0).r;
        yuv.y=texture2D(qt_Texture0,vec2(fxDD,qt_TexCoord0.t)).g-0.5;
        yuv.z=texture2D(qt_Texture0,qt_TexCoord0).g-0.5;
        rgb=mat3(1,1,1,\
                 0,-0.39465,2.03211,\
                 1.13983,-0.58060,0)*yuv;
*/
        yuv.x=texture2D(qt_Texture0,qt_TexCoord0).r;
        yuv.y=texture2D(qt_Texture0,vec2(fxDD,qt_TexCoord0.t)).g;
        yuv.z=texture2D(qt_Texture0,qt_TexCoord0).g;
        rgb.x = csp_coeff_0.x * yuv.x + csp_coeff_1.x * yuv.y + csp_coeff_2.x * yuv.z + csp_const.x;
        rgb.y = csp_coeff_0.y * yuv.x + csp_coeff_1.y * yuv.y + csp_coeff_2.y * yuv.z + csp_const.y;
        rgb.z = csp_coeff_0.z * yuv.x + csp_coeff_1.z * yuv.y + csp_coeff_2.z * yuv.z + csp_const.z;


        gl_FragData[0]=vec4(rgb,1);
    }
}
