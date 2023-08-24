#!/bin/sh

cd "$(dirname "$0")/."

echo "Updating pffft"
rm -rf ./pffft
curl -LO https://codeload.github.com/marton78/pffft/zip/refs/heads/master > /dev/null 2>&1
unzip ./master > /dev/null 2>&1
rm ./master
mv ./pffft-master ./pffft

echo "Updating audio convolver"
rm -rf ./fft_consolver
curl -LO https://github.com/falkTX/FFTConvolver/archive/refs/heads/non-uniform.zip > /dev/null 2>&1
unzip ./non-uniform.zip > /dev/null 2>&1
rm ./non-uniform.zip
mv ./FFTConvolver-non-uniform ./fft_consolver

echo "Updating qoa"
rm -rf ./qoa
curl -LO https://github.com/phoboslab/qoa/archive/refs/heads/master.zip > /dev/null 2>&1
unzip ./master.zip > /dev/null 2>&1
rm ./master.zip
mv ./qoa-master ./qoa

echo "Updating tracey"
rm -rf ./tracy
curl -LO https://github.com/wolfpld/tracy/archive/refs/heads/master.zip > /dev/null 2>&1
unzip ./master.zip > /dev/null 2>&1
rm ./master.zip
mv ./tracy-master ./tracy

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
# export macro creates some issues with iplug2 vst
sed -i 's/EXPORT\ //g' -b -i ./speex_resampler/resample.c
rm -rf ./speexdsp-master/

echo "Updating xsimd"
rm -rf ./xsimd
rm ./master
curl -LO https://codeload.github.com/xtensor-stack/xsimd/zip/refs/heads/master > /dev/null 2>&1
unzip ./master > /dev/null 2>&1
rm ./master
mv ./xsimd-master ./xsimd

echo "Updating dirent"
rm ./dirent.h
curl -LO https://raw.githubusercontent.com/tronkko/dirent/master/include/dirent.h > /dev/null 2>&1

echo "Updating dr_wav"
rm ./dr_wav.h
curl -LO https://raw.githubusercontent.com/mackron/dr_libs/master/dr_wav.h > /dev/null 2>&1

echo "Updating stb_vorbis"
rm ./stb_vorbis.c
curl -LO https://raw.githubusercontent.com/nothings/stb/master/stb_vorbis.c

echo "Updating stb_sprintf"
rm ./stb_sprintf.h
curl -LO https://raw.githubusercontent.com/nothings/stb/master/stb_sprintf.h

echo "Downloading hiir"
HIIR="hiir-1.40.zip"
rm -rf ./hiir
mkdir ./hiir
cd ./hiir
curl -LO http://ldesoras.free.fr/src/$HIIR > /dev/null 2>&1
unzip ./$HIIR > /dev/null 2>&1
rm ./$HIIR
cd ./hiir
# Make the includes relative
sed -i 's/include\ "hiir/include\ "./g' *.*
cd ../..
