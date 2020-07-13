# Maintainer: Hal Clark <gmail.com[at]hdeanclark>
pkgname=explicator
pkgver=20200415_113638
pkgver() {
  date +%Y%m%d_%H%M%S
}
pkgrel=1

pkgdesc="String translation library using a combination of string similarity metrics."
url="http://www.halclark.ca"
arch=('x86_64' 'i686' 'armv7h')
license=('GPL' 'FDL')
depends=('gcc-libs')
makedepends=('cmake')
# optdepends=()
# conflicts=()
# replaces=()
# backup=()
# install='foo.install'
source=("git+https://github.com/hdclark/explicator.git")
md5sums=('SKIP')
sha1sums=('SKIP')
# options=(strip staticlibs)

build() {
  # ---------------- Configure -------------------
  # Try use environment variable, but fallback to standard. 
  install_prefix=${INSTALL_PREFIX:-/usr}

  # Default build with default compiler flags.
  cmake \
    -DCMAKE_INSTALL_PREFIX="${install_prefix}" \
    -DCMAKE_BUILD_TYPE=Debug \
    "${srcdir}"/explicator
  make -j 2 VERBOSE=1
}

package() {
  make -j 2 DESTDIR="${pkgdir}" install
}

# vim:set ts=2 sw=2 et:
