ARCH_GET=`uname -m`
if [ "$ARCH_GET" = "x86_64" ];
then
	ARCH=x64
else
	ARCH=i386
fi
tar -zxf ffmpeg-3.3.3.tar.gz
cd ffmpeg-3.3.3
./configure --disable-debug --enable-static --disable-shared --disable-everything \
--disable-muxers --disable-muxer=ffm --disable-muxer=mov \
--disable-cuda --disable-nvenc --disable-vaapi --disable-cuvid \
--disable-demuxers --disable-demuxer=asf --disable-demuxer=mpegts \
--disable-demuxer=rtsp --disable-demuxer=mov --disable-demuxer=rm \
--disable-protocols --enable-protocol=file --disable-protocol=rtp \
--disable-protocol=udp --disable-protocol=http --disable-protocol=tcp \
--disable-filters --disable-filter=aformat --disable-filter=format \
--disable-filter=setpts --disable-filter=anull --disable-filter=hflip \
--disable-filter=transpose --disable-filter=atrim --disable-filter=null \
--disable-filter=trim --disable-filter=crop --disable-filter=rotate --disable-filter=vflip \
--enable-decoder=ac3 --enable-decoder=dca --enable-decoder=mp3 \
--enable-decoder=hevc --enable-decoder=h264 --enable-decoder=aac --enable-muxer=mp4 --enable-muxer=mov \
--enable-parser=hevc --enable-parser=h264 --enable-parser=aac --enable-parser=mpegaudio \
--enable-bsf=aac_adtstoasc --enable-bsf=mp3_header_decompress \
--disable-debug --prefix=./bulid --disable-asm
make
make install
cp -rf ./bulid/lib/* ../../ffmpeg/lib/$ARCH
