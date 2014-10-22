mesa-mesa
=========

Repo with changes to build with Android-x86
Forked from unofficial mirror of freedesktop mesa repository which is updated daily.

In order to build mesa 10.3.1 with Android-x86:

- replace external/drm with libdrm 2.4.58

- if you want to try experimental drm_gralloc_nouveau and add nouveau in device/generic/x86/BoardConfig.mk, replace hardware/drm_gralloc, using one of following gits:

https://github.com/evelikov/hardware_drm_gralloc or https://github.com/pstglia/drm_gralloc

- revert commits that introduced tr1/unordered_set, because STLport does not support all types in its own tr1/unordered_set implementation.

git revert ...
git revert ...
git revert ...
git revert ...

Limitations:

- Android.mk files not yet ready for supporting gallium radeonsi driver in device/generic/x86/BoardConfig.mk
- gallium nouveau can be built with tr1 code reverted, but still unstable apparently due to buffer objects invalid addresses and/or GPU locking, missing GPU registers initialization code.
- gallium r300, r600 can build and seem to be working on HD4xxx and HD5xxx, but still not extensively tested
