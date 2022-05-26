-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: openssl
Binary: openssl, libssl1.1, libcrypto1.1-udeb, libssl1.1-udeb, libssl-dev, libssl-doc
Architecture: any all
Version: 1.1.1n-0+deb10u1
Maintainer: Debian OpenSSL Team <pkg-openssl-devel@lists.alioth.debian.org>
Uploaders: Christoph Martin <christoph.martin@uni-mainz.de>, Kurt Roeckx <kurt@roeckx.be>, Sebastian Andrzej Siewior <sebastian@breakpoint.cc>
Homepage: https://www.openssl.org/
Standards-Version: 4.2.1
Vcs-Browser: https://salsa.debian.org/debian/openssl
Vcs-Git: https://salsa.debian.org/debian/openssl.git
Testsuite: autopkgtest
Testsuite-Triggers: perl
Build-Depends: debhelper (>= 11), m4, bc, dpkg-dev (>= 1.15.7)
Package-List:
 libcrypto1.1-udeb udeb debian-installer optional arch=any
 libssl-dev deb libdevel optional arch=any
 libssl-doc deb doc optional arch=all
 libssl1.1 deb libs optional arch=any
 libssl1.1-udeb udeb debian-installer optional arch=any
 openssl deb utils optional arch=any
Checksums-Sha1:
 4b0936dd798f60c97c68fc62b73033ecba6dfb0c 9850712 openssl_1.1.1n.orig.tar.gz
 7c47f949298c841ec022afffb8378de7179b9378 488 openssl_1.1.1n.orig.tar.gz.asc
 a30b8ed84c8f6e1be7636678e76959262e672cc8 84528 openssl_1.1.1n-0+deb10u1.debian.tar.xz
Checksums-Sha256:
 40dceb51a4f6a5275bde0e6bf20ef4b91bfc32ed57c0552e2e8e15463372b17a 9850712 openssl_1.1.1n.orig.tar.gz
 e0e89e9467102880ee6f2ee8c1413933eb1268969afb97b9bec61e2190a62fd0 488 openssl_1.1.1n.orig.tar.gz.asc
 921cf24cb0aa1f5a33ae39f0476d9aea2b56a17ca84b89a46999eb8b22af9129 84528 openssl_1.1.1n-0+deb10u1.debian.tar.xz
Files:
 2aad5635f9bb338bc2c6b7d19cbc9676 9850712 openssl_1.1.1n.orig.tar.gz
 55cfa25c4358a6e2f8bb6ad7dd0b9f93 488 openssl_1.1.1n.orig.tar.gz.asc
 f87b4cce4ca364f14586f0305f0c4b2c 84528 openssl_1.1.1n-0+deb10u1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEEZCVGlf/wqkRmzBnme5boFiqM9dEFAmI08KgACgkQe5boFiqM
9dHWsA//TULQQ01T6C2XaBiH/F4G721Um3skmL+e7s62JZW42+J7IZ38/gk17Auj
u4ufM5zuaMTGDlzQouJShtRkjEao3NgFlWB/epNjCvfH+uOP+DSfdeYEiMu8PK2E
M9SosymTjVy+hFr1GnYRfoMoHCePoMTlSA8EG1kt+Bt/x7z8s/GKMn0feWQ9pm3F
HZcgJKsdxHtfMm5LqwOUEh4X6tCKKFXwRs/YJhNj7wBwVh/qmo3qK3989VV24LWM
S9zzC7XQO2h1NJSijwMTghuG853UT33A3/kg4tFAlnxiIc++tIV4tJK3NvZMu73m
iyPwm7dNJZtii940QNOTFE/1QlvURyNC58ByfIFGo7ZRJN4Z+RmNy9xP1VxuzxQq
il0Pb+TEg0TauQDau4JVFEhxFnH06ayD6v9Qr/uYhMHDwRwzuzIpf4EnjK+ytySt
Rd0sb2Nwz6REuspeT7/Qjzquih/Kmq8108LRsMpgbg0f2Qtv+A/cTGWSZOSjbNSf
+ciyeTDShVrqpjpRna9/OGY/aShiaOp7F1FhzC9jhZ/P2XRxSg9FtqfRS6KG4zn3
NivoZ2y0+U/j/V7uxvVfkL54VnapDq4Y6zsSzVsHuvfmgOd5k8rf+ePNkaAjDRok
Y5sBK6NPfIzGioMAkEwAqDZUF3oi9IDzG5+H0rjPIqmR64N3G4Q=
=5Wvq
-----END PGP SIGNATURE-----
