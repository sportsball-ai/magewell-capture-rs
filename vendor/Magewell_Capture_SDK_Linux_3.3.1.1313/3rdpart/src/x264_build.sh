ARCH_GET=`uname -m`
if [ "$ARCH_GET" = "x86_64" ];
then
	ARCH=x64
else
	ARCH=i386
fi
tar -jxvf x264-snapshot-20170101-2245.tar.bz2
cd x264-snapshot-20170101-2245
./configure 
make
cp -rf ./libx264.a ../../x264/lib/$ARCH
