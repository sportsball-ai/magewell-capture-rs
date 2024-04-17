package com.magewell;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;
import java.nio.ByteBuffer;
import javax.swing.*;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.opengl.GLCanvas;
import org.eclipse.swt.opengl.GLData;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;
import org.lwjgl.opengl.GL;

public class AVCapture extends JFrame{
    int CAPTURE_WIDTH            = 1920;
    int CAPTURE_HEIGHT           = 1080;
    long CAPTURE_FOURCC           = LibMWCapture.MWFOURCC_NV12;

    int CAPTURE_CHANNEL          = 2;
    int CAPTURE_SAMPLE_RATE      = 48000;
    int CAPTURE_BIT_PER_SAMPLE   = 16;


    int m_select_device_index = 0;
    int m_is_capturing = 0;
    Capture capture;

    int m_video_render_frames = 0;
    int m_audio_play_frames = 0;


	class AudioPlayerThread implements Runnable {
		@Override
		public void run() {
            try {
                AudioFormat af = new AudioFormat(CAPTURE_SAMPLE_RATE, CAPTURE_BIT_PER_SAMPLE, CAPTURE_CHANNEL, true, false);
                SourceDataLine.Info info = new DataLine.Info(SourceDataLine.class, af, CAPTURE_CHANNEL*CAPTURE_BIT_PER_SAMPLE/2*LibMWCapture.MWCAP_AUDIO_SAMPLES_PER_FRAME);
                SourceDataLine sdl = (SourceDataLine) AudioSystem.getLine(info);
                sdl.open(af);
                sdl.start();
                while(0 != m_is_capturing){
                    byte[] pcm = capture.get_audio_play();
                    if(null == pcm){
                        try {
                            Thread.sleep(5);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        continue;
                    }
                    sdl.write(pcm, 0, pcm.length);
                    m_audio_play_frames++;
                }
            } catch (LineUnavailableException e) {
                e.printStackTrace();
            }
        }
    }

    public AVCapture(String[] args) {
        parse_args(args);
        capture = new Capture();
        capture.set_video(CAPTURE_FOURCC, CAPTURE_WIDTH, CAPTURE_HEIGHT);
        capture.set_audio(CAPTURE_CHANNEL, CAPTURE_SAMPLE_RATE, CAPTURE_BIT_PER_SAMPLE);
        int ret = capture.start_capture(m_select_device_index);
        if(ret < 0){
            System.out.println("can not start capture");
            return;
        }
        m_is_capturing = 1;

        Thread m_audio_player_thread;
        m_audio_player_thread = new Thread(new AudioPlayerThread());
        System.out.println("Audio Player Thread: " + m_audio_player_thread);
        m_audio_player_thread.setPriority(Thread.MAX_PRIORITY);
        m_audio_player_thread.start();

        Display display = new Display();
        Shell shell = new Shell(display);
        shell.setLayout(new FillLayout());
        shell.addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                if (e.stateMask == SWT.CTRL && e.keyCode <= '9' && e.keyCode >= '0' && (e.keyCode - '0' != m_select_device_index)) {
                    m_select_device_index = e.keyCode - '0';
                    capture.stop_capture();
                    capture.start_capture(m_select_device_index);
                    if(ret < 0){
                        System.out.println("can not start capture");
                        return;
                    }
                    //shell.setFullScreen(!shell.getFullScreen());
                }else if(e.stateMask == SWT.ALT && e.keyCode == 'r'){//start record
                }else if(e.stateMask == SWT.ALT && e.keyCode == 's'){//stop record
                }
            }
        });

        shell.addListener(SWT.Traverse, new Listener() {
            public void handleEvent(Event event) {
                switch (event.detail) {
                    case SWT.TRAVERSE_ESCAPE:
                        shell.close();
                        event.detail = SWT.TRAVERSE_NONE;
                        event.doit = false;
                        break;
                    default:
                        break;
                }
            }
        });

        //shell.setMinimumSize(320, 180);
        GLData data = new GLData();
        data.doubleBuffer = true;
        GLCanvas canvas = new GLCanvas(shell, SWT.NO_BACKGROUND | SWT.NO_REDRAW_RESIZE, data);
        canvas.setCurrent();
        GL.createCapabilities();
        final Rectangle rect = new Rectangle(0, 0, 0, 0);
        canvas.addListener(SWT.Resize, new Listener() {
            public void handleEvent(Event event) {
                Rectangle bounds = canvas.getBounds();
                rect.width = bounds.width;
                rect.height = bounds.height;
            }
        });
        shell.setSize(960, 540);
        shell.open();
        //mw_opengl_render render = new mw_opengl_render();
        //render.open_render(CAPTURE_WIDTH, CAPTURE_HEIGHT);
        VideoRender render = new VideoRender();
        //ByteBuffer render_buf = capture.get_video_play();//
        //ByteBuffer render_buf =ByteBuffer.allocateDirect((int)1920*1080*2);
        //render_buf.clear();
        render.init(get_render_fourcc(),CAPTURE_WIDTH, CAPTURE_HEIGHT);
        display.asyncExec(new Runnable() {
            public void run() {
                if (!canvas.isDisposed()) {
                    //LibMWCapture.GetFrame(render_buf);
                    ByteBuffer render_buf = capture.get_video_play();
                    if(null == render_buf){
                        try {
                            Thread.sleep(5);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }else{
                        //render.render_frame(render_buf, rect.width,rect.height, false);
                        render.render(render_buf, rect.width,rect.height);
                        canvas.swapBuffers();
                        m_video_render_frames++;
                    }
                    print_fps();
                    display.asyncExec(this);
                }
            }
        });
        while (!shell.isDisposed()) {
            if (!display.readAndDispatch())
                display.sleep();
        }
        display.dispose();
        shell.dispose();
        canvas.dispose();
        capture.stop_capture();
        m_is_capturing = 0;
        try {
            m_audio_player_thread.join();
        } catch (InterruptedException e) {
            System.out.println("Audio Play Thread Interrupt fail !");
            e.printStackTrace();
        }
	}
    int m_prev_video_render_frames = 0;
    int m_prev_audio_play_frames = 0;
    int m_prev_video_capture_frames = 0;
    int m_prev_audio_capture_frames = 0;
    long m_prev_times = 0;
    int m_count = 0;

    public void print_fps()
    {
        m_count++;
        if(m_count == 1){
            m_prev_times = System.currentTimeMillis();
            m_prev_video_render_frames = m_video_render_frames;
            m_prev_audio_play_frames = m_audio_play_frames;
            m_prev_video_capture_frames = capture.get_video_capture_frames();
            m_prev_audio_capture_frames = capture.get_audio_capture_frames();
            return;
        }else if((m_count % 1000) != 99){
            return;
        }
        long now_time = System.currentTimeMillis();
        if(now_time == m_prev_times){
            return;
        }
        double audio_capture_fps = (capture.get_audio_capture_frames() - m_prev_audio_capture_frames) * 1000.0/(now_time-m_prev_times);
        double audio_play_fps = (m_audio_play_frames - m_prev_audio_play_frames) * 1000.0/(now_time-m_prev_times);
        double video_capture_fps = (capture.get_video_capture_frames() - m_prev_video_capture_frames) * 1000.0/(now_time-m_prev_times);
        double video_render_fps = (m_video_render_frames - m_prev_video_render_frames) * 1000.0/(now_time-m_prev_times);

        m_prev_video_render_frames = m_video_render_frames;
        m_prev_audio_play_frames = m_audio_play_frames;
        m_prev_video_capture_frames = capture.get_video_capture_frames();
        m_prev_audio_capture_frames = capture.get_audio_capture_frames();
        m_prev_times = now_time;

        System.out.println("capture fps(v|a):"+video_capture_fps+"|"+audio_capture_fps +"capture frams(v|a):"+capture.get_video_capture_frames()+"|"+capture.get_audio_capture_frames());
        System.out.println("render fps(v|a):"+video_render_fps+"|"+audio_play_fps +"render frams(v|a):"+m_video_render_frames+"|"+m_audio_play_frames+"\n");
    }
    public int get_render_fourcc()
    {
        if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_RGB24){
            return VideoRender.RENDER_RGB24;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_BGR24){
            return VideoRender.RENDER_BGR24;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_I420){
            return VideoRender.RENDER_I420;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_YV12){
            return VideoRender.RENDER_YV12;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_NV12){
            return VideoRender.RENDER_NV12;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_NV21){
            return VideoRender.RENDER_NV21;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_P010){
            return VideoRender.RENDER_P010;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_YUY2){
            return VideoRender.RENDER_YUY2;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_RGBA){
            return VideoRender.RENDER_RGBA;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_BGRA){
            return VideoRender.RENDER_BGRA;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_ARGB){
            return VideoRender.RENDER_ARGB;
        }
        else if(CAPTURE_FOURCC == LibMWCapture.MWFOURCC_ABGR){
            return VideoRender.RENDER_ABGR;
        }
        return VideoRender.RENDER_NV12;
    }
    public void parse_args(String[] args){
        for(int i = 0; i < args.length-1; i += 2){
            if(args[i].equals("-width")){
                CAPTURE_WIDTH = Integer.parseInt(args[i+1]);
                System.out.println("width set to "+CAPTURE_WIDTH);
            }else if(args[i].equals("-height")){
                CAPTURE_HEIGHT = Integer.parseInt(args[i+1]);
                System.out.println("height set to "+CAPTURE_HEIGHT);
            }else if(args[i].equals("-fourcc")){
                if(args[i+1].equals("nv12")){
                    CAPTURE_FOURCC  = LibMWCapture.MWFOURCC_NV12;
                    System.out.println("fourcc set to nv12");
                }else if(args[i+1].equals("yv12")){
                    CAPTURE_FOURCC  = LibMWCapture.MWFOURCC_YV12;
                    System.out.println("fourcc set to yv12");
                }else if(args[i+1].equals("yuy2")){
                    CAPTURE_FOURCC  = LibMWCapture.MWFOURCC_YUY2;
                    System.out.println("fourcc set to yuy2");
                }else if(args[i+1].equals("rgb")){
                    CAPTURE_FOURCC  = LibMWCapture.MWFOURCC_RGB24;
                    System.out.println("fourcc set to rgb24");
                }else if(args[i+1].equals("rgba")){
                    CAPTURE_FOURCC  = LibMWCapture.MWFOURCC_RGBA;
                    System.out.println("fourcc set to rgba");
                }else if(args[i+1].equals("argb")){
                    CAPTURE_FOURCC  = LibMWCapture.MWFOURCC_ARGB;
                    System.out.println("fourcc set to argb");
                }else{
                    System.out.println("fourcc set fail"+args[i+1]);
                }
            }else if(args[i].equals("-sample_rate")){
                CAPTURE_SAMPLE_RATE = Integer.parseInt(args[i+1]);
                System.out.println("sample_rate set to "+CAPTURE_SAMPLE_RATE);
            }
        }
    }
	public static void main(String[] args) {
        new AVCapture(args);
        System.out.println("exit");
        System.gc();
	}
}
