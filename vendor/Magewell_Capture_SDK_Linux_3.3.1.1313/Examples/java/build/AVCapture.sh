export LD_LIBRARY_PATH=../jar/jni:$LD_LIBRARY_PATH
rm -rf com
cd ../jar/jni
make clean
make
#libMWCaptureJni.a
cd ../
rm ./com/magewell/*.class
rm libMWCapture.jar
#mkdir my
javac -d . ./com/magewell/*.java

jar -cvf libMWCapture.jar ./com/magewell/*.class
cd ../bulid
rm -rf com/magewell/*.class
javac -classpath ../jar/libMWCapture.jar:../jar/org/lwjgl/lwjgl.jar:../jar/org/lwjgl/lwjgl-natives-linux.jar:../jar/org/lwjgl-opengl/lwjgl-opengl.jar:../jar/org/lwjgl-opengl/lwjgl-opengl-natives-linux.jar:../jar/org/SWT.jar -d . ../AVCapture/com/magewell/*.java
java -cp .:../jar/libMWCapture.jar:../jar/org/lwjgl/lwjgl.jar:../jar/org/lwjgl/lwjgl-natives-linux.jar:../jar/org/lwjgl-opengl/lwjgl-opengl.jar:../jar/org/lwjgl-opengl/lwjgl-opengl-natives-linux.jar:../jar/org/SWT.jar com.magewell.AVCapture -width 1920 -height 1080 -fourcc yuy2 -sample_rate 48000
