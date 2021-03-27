#!/bin/sh

echo "Updating xsimd"
rm -rf ./xsimd
rm ./master
curl -LO https://codeload.github.com/xtensor-stack/xsimd/zip/refs/heads/master > /dev/null 2>&1
unzip ./master > /dev/null 2>&1
rm ./master
mv ./xsimd-master ./xsimd

echo "Updating pffft"
rm -rf ./pffft
curl -LO https://codeload.github.com/marton78/pffft/zip/refs/heads/master > /dev/null 2>&1
unzip ./master > /dev/null 2>&1
rm ./master
mv ./pffft-master ./pffft

echo "Updating speex_resampler"
rm -rf ./speex_resampler
curl -LO https://codeload.github.com/xiph/speexdsp/zip/refs/heads/master > /dev/null 2>&1
unzip ./master > /dev/null 2>&1
rm ./master
mkdir ./speex_resampler
mv ./speexdsp-master/libspeexdsp/arch.h ./speex_resampler/arch.h
mv ./speexdsp-master/libspeexdsp/os_support.h ./speex_resampler/os_support.h
mv ./speexdsp-master/libspeexdsp/resample* ./speex_resampler/
mv ./speexdsp-master/include/speex/speex_resampler.h ./speex_resampler/speex_resampler.h
# When running outside speex it's apparently not possible to define own
# Memory allocation routines, we just comment them out
sed -e '/static void \*speex_alloc/ s/^\/*/\/\//' -i ./speex_resampler/resample.c
sed -e '/static void \*speex_realloc/ s/^\/*/\/\//' -i ./speex_resampler/resample.c
sed -e '/static void speex_free/ s/^\/*/\/\//' -i ./speex_resampler/resample.c
sed -i 's/EXPORT\ //g' -b -i ./speex_resampler/resample.c
rm -rf ./speexdsp-master/

echo "Updating dirent"
rm ./dirent.h
curl -LO https://raw.githubusercontent.com/tronkko/dirent/master/include/dirent.h > /dev/null 2>&1

echo "Updating dr_wav"
rm ./dr_wav.h
curl -LO https://raw.githubusercontent.com/mackron/dr_libs/master/dr_wav.h > /dev/null 2>&1

echo "Downloading hiir"
HIIR="hiir-1.33.zip"
rm -rf ./hiir
mkdir ./hiir
cd ./hiir
curl -LO http://ldesoras.free.fr/src/$HIIR > /dev/null 2>&1
unzip ./$HIIR > /dev/null 2>&1
rm ./$HIIR
cd ./hiir
sed -i 's/include\ "hiir/include\ "./g' *
cd ../..