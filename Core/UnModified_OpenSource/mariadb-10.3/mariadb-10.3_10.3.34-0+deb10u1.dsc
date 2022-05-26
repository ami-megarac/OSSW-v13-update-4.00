-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: mariadb-10.3
Binary: libmariadb-dev, libmariadbclient-dev, libmariadb-dev-compat, libmariadb3, libmariadbd19, libmariadbd-dev, mariadb-common, mariadb-client-core-10.3, mariadb-client-10.3, mariadb-server-core-10.3, mariadb-server-10.3, mariadb-server, mariadb-client, mariadb-backup, mariadb-plugin-connect, mariadb-plugin-rocksdb, mariadb-plugin-oqgraph, mariadb-plugin-tokudb, mariadb-plugin-mroonga, mariadb-plugin-spider, mariadb-plugin-gssapi-server, mariadb-plugin-gssapi-client, mariadb-plugin-cracklib-password-check, mariadb-test, mariadb-test-data
Architecture: any all
Version: 1:10.3.34-0+deb10u1
Maintainer: Debian MySQL Maintainers <pkg-mysql-maint@lists.alioth.debian.org>
Uploaders: Otto Kekäläinen <otto@debian.org>
Homepage: https://mariadb.org/
Standards-Version: 4.2.1
Vcs-Browser: https://salsa.debian.org/mariadb-team/mariadb-10.3
Vcs-Git: https://salsa.debian.org/mariadb-team/mariadb-10.3.git
Testsuite: autopkgtest
Build-Depends: bison, chrpath, cmake, cracklib-runtime, debhelper (>= 9.20160709), dh-apparmor, dh-exec, gdb, libaio-dev [linux-any], libarchive-dev, libboost-dev, libcrack2-dev (>= 2.9.0), libgnutls28-dev (>= 3.3.24), libjemalloc-dev [linux-any], libjudy-dev, libkrb5-dev, liblz4-dev, libncurses5-dev (>= 5.0-6~), libpam0g-dev, libpcre3-dev (>= 2:8.35-3.2~), libreadline-gplv2-dev, libreadline-gplv2-dev:native, libsnappy-dev, libsystemd-dev [linux-any], libxml2-dev, libzstd-dev (>= 1.3.3), lsb-release, perl, po-debconf, psmisc, unixodbc-dev, zlib1g-dev (>= 1:1.1.3-5~)
Package-List:
 libmariadb-dev deb libdevel optional arch=any
 libmariadb-dev-compat deb libdevel optional arch=any
 libmariadb3 deb libs optional arch=any
 libmariadbclient-dev deb oldlibs optional arch=any
 libmariadbd-dev deb libdevel optional arch=any
 libmariadbd19 deb libs optional arch=any
 mariadb-backup deb database optional arch=any
 mariadb-client deb database optional arch=all
 mariadb-client-10.3 deb database optional arch=any
 mariadb-client-core-10.3 deb database optional arch=any
 mariadb-common deb database optional arch=all
 mariadb-plugin-connect deb database optional arch=any
 mariadb-plugin-cracklib-password-check deb database optional arch=any
 mariadb-plugin-gssapi-client deb database optional arch=any
 mariadb-plugin-gssapi-server deb database optional arch=any
 mariadb-plugin-mroonga deb database optional arch=any-alpha,any-amd64,any-arm,any-arm64,any-i386,any-ia64,any-mips64el,any-mips64r6el,any-mipsel,any-mipsr6el,any-nios2,any-powerpcel,any-ppc64el,any-sh3,any-sh4,any-tilegx
 mariadb-plugin-oqgraph deb database optional arch=any
 mariadb-plugin-rocksdb deb database optional arch=amd64,arm64,mips64el,ppc64el
 mariadb-plugin-spider deb database optional arch=any
 mariadb-plugin-tokudb deb database optional arch=amd64
 mariadb-server deb database optional arch=all
 mariadb-server-10.3 deb database optional arch=any
 mariadb-server-core-10.3 deb database optional arch=any
 mariadb-test deb database optional arch=any
 mariadb-test-data deb database optional arch=all
Checksums-Sha1:
 4084b765b40de13dfee27fd064cf227a4e2149c9 73455446 mariadb-10.3_10.3.34.orig.tar.gz
 92e6292440f797cbb03c9633daa25c1a3c1de478 195 mariadb-10.3_10.3.34.orig.tar.gz.asc
 006cde700aa2b133fffa92dac3172cd9d1958e85 219456 mariadb-10.3_10.3.34-0+deb10u1.debian.tar.xz
Checksums-Sha256:
 7b7013448519d20590ec90eac54ca54e9ac9c4b7d90eaacb7451650e38be0627 73455446 mariadb-10.3_10.3.34.orig.tar.gz
 ad7273dcc79125d3d4b6e60c636e4cbc023c993bf1d744f5852ea3a128f3cf1f 195 mariadb-10.3_10.3.34.orig.tar.gz.asc
 b8d8da3e2d1670c1852933a71c7692bde1a7c6427db8e199746725e9988319fd 219456 mariadb-10.3_10.3.34-0+deb10u1.debian.tar.xz
Files:
 9025c224552a3dce13f1ae5269add9d3 73455446 mariadb-10.3_10.3.34.orig.tar.gz
 24b5fcc29eea1afde9b3cb555ee105e1 195 mariadb-10.3_10.3.34.orig.tar.gz.asc
 b5891286930288dd2daf205ef0915b64 219456 mariadb-10.3_10.3.34-0+deb10u1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEEmbRSsR88dMO0U+RvvthEn87o2ogFAmIe/wwACgkQvthEn87o
2ohP4hAAluJqJQlsZckBJAZGQ6bimcQTG8ljzr8oyL9DXs4BBCCLtV+/OQYvJCPM
XzIfbWLGnO1Lc5EKi51EoH+SlpIMlR/2zBfeYTiPd59HaqUPjptEEaIDZO20CoO1
e/ElyFR8btch0pUZ1Vfa5j8tt7f3P8fQGLxwJfkGcCCAPH5lNGZrOJoCOzgU8x/F
Rlip4zs/Id4y7Tz5E2ENremSZW4lNb285nwa2MVF+V9ZmqooIxd9IYsb7Jer1wgH
/wNsqOBVk7zqOy9oIYuL/I6A/ar7mo7tcIWPkNU8ur7qd7yXVNBy+XeUOQtWO7AH
hYclfLCoHmH1YSlVtR9djw5hheWOFzDzqdA81mnMevZ2+C5Zj6ryQLTUuAjlrClF
5h/tdBDHGfepOko6RRuPLDjGiLQFMoSjnGIEf3jH1j5NmyhjToZuTQ+kqCM63Jxc
9kxMYqbL76GRYv3FejdlXdUCt1n8CsnsfUukCVY8JjdYCoHZ0EysR8qQxwIJuchN
gQ6DDNRAMix/gDfKd/tDHT0ZAQkukuzi+486nozGTi1W1gG/LEKuwTRW1Sq6lJB+
Nzvs4DJ78uY3TcNBSmDHc/OXHUYRHb/0AG4l3Y1TD4PDzHasCU7t13+AZgADtcm3
yv1PAI+LkY5Asao1VGKBqlmn2GuTvSAyP3DGRPUkQ7+H+Aq1BeE=
=ObrH
-----END PGP SIGNATURE-----
