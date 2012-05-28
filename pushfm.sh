# !/bin/sh
cd ~/work/android-src
. build/envsetup.sh
cd fm
mm

echo "push fm.so"
adb push ../out/target/product/generic/obj/lib/fm.so /system/lib/fm.so
echo "push fm"
adb push ../out/target/product/generic/system/bin/fm /system/bin/fm

# adb shell fm
