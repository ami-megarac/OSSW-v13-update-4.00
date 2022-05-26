-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: cyrus-sasl2
Binary: sasl2-bin, cyrus-sasl2-doc, libsasl2-2, libsasl2-modules, libsasl2-modules-db, libsasl2-modules-ldap, libsasl2-modules-otp, libsasl2-modules-sql, libsasl2-modules-gssapi-mit, libsasl2-dev, libsasl2-modules-gssapi-heimdal
Architecture: any all
Version: 2.1.27+dfsg-1+deb10u2
Maintainer: Debian Cyrus Team <team+cyrus@tracker.debian.org>
Uploaders: Fabian Fagerholm <fabbe@debian.org>, Roberto C. Sanchez <roberto@debian.org>, Ondřej Surý <ondrej@debian.org>, Adam Conrad <adconrad@0c3.net>
Homepage: https://www.cyrusimap.org/sasl/
Standards-Version: 4.3.0.1
Vcs-Browser: https://salsa.debian.org/debian/cyrus-sasl2
Vcs-Git: https://salsa.debian.org/debian/cyrus-sasl2.git
Build-Depends: chrpath, debhelper-compat (= 12), default-libmysqlclient-dev <!pkg.cyrus-sasl2.nosql> | libmysqlclient-dev <!pkg.cyrus-sasl2.nosql>, docbook-to-man, groff-base, heimdal-multidev <!pkg.cyrus-sasl2.nogssapi>, krb5-multidev <!pkg.cyrus-sasl2.nogssapi>, libdb-dev, libkrb5-dev <!pkg.cyrus-sasl2.nogssapi>, libldap2-dev <!pkg.cyrus-sasl2.noldap>, libpam0g-dev, libpod-pom-view-restructured-perl, libpq-dev <!pkg.cyrus-sasl2.nosql>, libsqlite3-dev, libssl-dev, po-debconf, python3-sphinx, quilt
Build-Conflicts: heimdal-dev
Package-List:
 cyrus-sasl2-doc deb doc optional arch=all
 libsasl2-2 deb libs standard arch=any
 libsasl2-dev deb libdevel optional arch=any
 libsasl2-modules deb libs optional arch=any
 libsasl2-modules-db deb libs standard arch=any
 libsasl2-modules-gssapi-heimdal deb libs optional arch=any profile=!pkg.cyrus-sasl2.nogssapi
 libsasl2-modules-gssapi-mit deb libs optional arch=any profile=!pkg.cyrus-sasl2.nogssapi
 libsasl2-modules-ldap deb libs optional arch=any profile=!pkg.cyrus-sasl2.noldap
 libsasl2-modules-otp deb libs optional arch=any
 libsasl2-modules-sql deb libs optional arch=any profile=!pkg.cyrus-sasl2.nosql
 sasl2-bin deb utils optional arch=any
Checksums-Sha1:
 6da3baff1685e96b93b46cdd47e13ecc34a632df 2058596 cyrus-sasl2_2.1.27+dfsg.orig.tar.xz
 fdc76f2d21432908d02dcccd4b0157d0abb02611 101064 cyrus-sasl2_2.1.27+dfsg-1+deb10u2.debian.tar.xz
Checksums-Sha256:
 108b0c691c423837264f05abb559ea76c3dfdd91246555e8abe87c129a6e37cd 2058596 cyrus-sasl2_2.1.27+dfsg.orig.tar.xz
 43296455617072928e3c409b3a40b08e98ad335b98fd40fc2a0d921bab72cf97 101064 cyrus-sasl2_2.1.27+dfsg-1+deb10u2.debian.tar.xz
Files:
 ce30955361d1cdde3c31d0ee742e338d 2058596 cyrus-sasl2_2.1.27+dfsg.orig.tar.xz
 1d79bbeb56624229395f148cf506a0d3 101064 cyrus-sasl2_2.1.27+dfsg-1+deb10u2.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQKmBAEBCgCQFiEERkRAmAjBceBVMd3uBUy48xNDz0QFAmIYlwlfFIAAAAAALgAo
aXNzdWVyLWZwckBub3RhdGlvbnMub3BlbnBncC5maWZ0aGhvcnNlbWFuLm5ldDQ2
NDQ0MDk4MDhDMTcxRTA1NTMxRERFRTA1NENCOEYzMTM0M0NGNDQSHGNhcm5pbEBk
ZWJpYW4ub3JnAAoJEAVMuPMTQ89EGMUP/1/c5/MEEcAWoU2IxofmP6KoKtWXk3k6
8S2hc/U2+kxxzFHnJgJ8Hb6XkvLr4nothlllj1wTFC/K8oSletb7MNUrf2VU3xjs
FFrBiaEhh1s4EmgpKfc2uzSlpj1Sp+jBsPqWMIHBgkwUBUBTmsF3eUxyNUoZRcq2
eUMMUaB2Sg8eCKAEf5VmP53i7fVaV/TWIcINCHApkFYt1j942DBMX6+QpXFFlNdF
y2jSP9xEEAhtaKF0zi0k/SqxZzzz3eKm6k6r08rIkawnNqzRRz54SqVAfZgq1JW6
BToTZ9ooaEpNuFrUyVitngp85ZfFOe2GUsfHCq7CZjMJ3K4GcJwPLMqA9K3euFrk
4uGxMbltmVRmtq+sbQ1kCZvZKTRqj/eyE2s8x9vBC0M/gtLAIFn9uKqVUL3pguC2
wCl7X2pXLk604WBa09Puwt6b4Fm4ZuNggABgH/tZwhzTizkhEvyg8pEP0nqxSUyA
1R6uvyjif7VABTjQjDopqWlI3qUkqC5OlYeCoezOeDWEFHX2pW3W9HJFhrICMmpQ
GR3xvH3R9SVopiKrKb/2TceXjIoU68wNBraCrzmh0lROGf6V4zaqqkQ2RBa5ZaxK
tGhZ7/NrKF783yOxMtaEa/HrDDOaxS6ZBhQqDlDlGhALsRNNMdb6CD6OpG+L1AfE
glAEtOS/f0Hx
=QJJO
-----END PGP SIGNATURE-----
