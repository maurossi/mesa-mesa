Repo with changes to build android_x86 and android_x86_64 targets

Forked from unofficial mirror of freedesktop mesa repository, which is updated daily.

There are several commits to Android.mk files but some one of them is an hack to avoid problems with Android Build System. generation of format_info.h instead of format_info.c accordingly to previous commit,
like renaming format_info.c to format_info.h

In order to build android_x86 and android_x86_64:

1) you'll need to download and replace following files in src/mesa/util folder:

http://www.mediafire.com/download/v80zdvc4atqh5g8/Android.mk
http://www.mediafire.com/view/xd1ccujwsri33po/format_srgb.c

NOUVEAU *EXPERIMENTAL*
If you want to try experimental drm_gralloc_nouveau and add nouveau in device/generic/x86/BoardConfig.mk, you can use the following github  hardware/drm_gralloc, using one of following gits:

https://github.com/maurossi/hardware_drm_gralloc

2) revert commits that introduced tr1/unordered_set, because STLport does not support all needed types
in its own tr1/unordered_set implementation and gnu_stl, libc++ or boost libs integration not yet successful.

git revert fb4e23626f9f27c62aa1cbea05a1c8b088fcf49a

git revert bb06f2cd93e8e16798bd59b458abd8d33a8e4658

git revert eaa9e14ce5a3e69c6aa303f2e5f039c1cc0b8de1

git revert a9b21015f5e3a6a37e53a8b3c755519f7b70479e


Limitations:

gallium r300, r600 can build and seem to be working on HD4xxx and HD5xxx, but still not extensively tested
gallium radeonsi building not yet achieved for Android-x86
gallium nouveau can be built, provided that tr1 code is reverted, because of STLport lack of supporting size_t
drm_gralloc_nouveau is still unstable apparently due to buffer objects invalid addresses and/or GPU locking, missing GPU registers initialization code.
intel i915, i965 classic drivers not working properly (GUI is there, but with heavy geometry artifacts). 
ilo may work (not tested), while i915g gallium driver (not tested) should be still veeery slow.

Any feedback/suggestion is appreciated expecially on tr1/unordered_set compiling with current AOSP toolchain or Android NDK latest GCC versions.

Mauro
