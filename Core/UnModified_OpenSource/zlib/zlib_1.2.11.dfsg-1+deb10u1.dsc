-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: zlib
Binary: zlib1g, zlib1g-dev, zlib1g-dbg, zlib1g-udeb, lib64z1, lib64z1-dev, lib32z1, lib32z1-dev, libn32z1, libn32z1-dev
Architecture: any
Version: 1:1.2.11.dfsg-1+deb10u1
Maintainer: Mark Brown <broonie@debian.org>
Homepage: http://zlib.net/
Standards-Version: 3.9.8
Build-Depends: debhelper (>= 8.1.3~), binutils (>= 2.18.1~cvs20080103-2) [mips mipsel], gcc-multilib [amd64 i386 kfreebsd-amd64 mips mipsel powerpc ppc64 s390 sparc s390x] <!stage1>, dpkg-dev (>= 1.16.1)
Package-List:
 lib32z1 deb libs optional arch=amd64,ppc64,kfreebsd-amd64,s390x profile=!stage1
 lib32z1-dev deb libdevel optional arch=amd64,ppc64,kfreebsd-amd64,s390x profile=!stage1
 lib64z1 deb libs optional arch=sparc,s390,i386,powerpc,mips,mipsel profile=!stage1
 lib64z1-dev deb libdevel optional arch=sparc,s390,i386,powerpc,mips,mipsel profile=!stage1
 libn32z1 deb libs optional arch=mips,mipsel profile=!stage1
 libn32z1-dev deb libdevel optional arch=mips,mipsel profile=!stage1
 zlib1g deb libs required arch=any
 zlib1g-dbg deb debug extra arch=any
 zlib1g-dev deb libdevel optional arch=any
 zlib1g-udeb udeb debian-installer optional arch=any
Checksums-Sha1:
 1b7f6963ccfb7262a6c9d88894d3a30ff2bf2e23 370248 zlib_1.2.11.dfsg.orig.tar.gz
 e35534fa3637a4ec0787ddfa5fbaa784bca5fe35 23092 zlib_1.2.11.dfsg-1+deb10u1.debian.tar.xz
Checksums-Sha256:
 80c481411a4fe8463aeb8270149a0e80bb9eaf7da44132b6e16f2b5af01bc899 370248 zlib_1.2.11.dfsg.orig.tar.gz
 eb26660e5b8a39f945a4fe1284e29b0279ded3513327e3cbd51c51921758f13f 23092 zlib_1.2.11.dfsg-1+deb10u1.debian.tar.xz
Files:
 2950b229ed4a5e556ad6581580e4ab2c 370248 zlib_1.2.11.dfsg.orig.tar.gz
 82dac8e2b7814db97e11f5675a6bcdc1 23092 zlib_1.2.11.dfsg-1+deb10u1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQKmBAEBCgCQFiEERkRAmAjBceBVMd3uBUy48xNDz0QFAmJBzoFfFIAAAAAALgAo
aXNzdWVyLWZwckBub3RhdGlvbnMub3BlbnBncC5maWZ0aGhvcnNlbWFuLm5ldDQ2
NDQ0MDk4MDhDMTcxRTA1NTMxRERFRTA1NENCOEYzMTM0M0NGNDQSHGNhcm5pbEBk
ZWJpYW4ub3JnAAoJEAVMuPMTQ89ExMcP+wZpHkS9O9wl9ETwoniiHWNTqETLdPus
rG02/y8nagqn0i9dvcygplzb2EfYsZYvM5Kt4k5dLPXKhWMpL59ULGWWVFWNSCTm
iY+NMjuq+hU8nUbkK/NbVOT97I7/Uu4AMezOO03gcxEMxOtdlukKyoKY9MtmD2LR
U/gMEJ+163ZlefhzWuFG327gQiUTvP/o/PG44QCudy0WAEEmfodlIHUfVeWW5kOr
o0r84dDF+K8aTRuDfsgZszOJz1BFDCarlcvf+OLwHzr6zopQ+VM3rHQXJoOW1BXO
rr1uUWOJ5FHAD0KuAOClLhCAewPtHs0uuiMOfhbhPRwFoKL0n96a0MGyblWKEvD8
lFIujz6vEDeB8s00xJMWlTMHEYfTV8mwf4ceBku8/PDqlwI1LQfCuVhmjSBWU3aN
o+3BDMd9L+8BMtzwALpQPF3nwlQSGHjv5Cj3QCD3ve2pFEsTneUqBfiA8B7XoJgf
oOH0cPOY1SSvRAOb1wIZUcFIYV5CbWRT9hyM1j1njivEB/iBNU5x2ahGuaEm2zd7
SG023oMu4HsD5S5ecTzJc0Lvp/Rcdmiso/UzHef44VSjcfU+zg4MzpQMHAUSM9Mj
J84aJkeamubBuRltypn3OKse1ZsGi91vvhXuIKKgnuAi4zwBSmO3goSpvwGEHaYJ
LQe2k+lAyvdJ
=ElAl
-----END PGP SIGNATURE-----
