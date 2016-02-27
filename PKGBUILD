# Maintainer: Hal Clark <gmail.com[at]hdeanclark>
pkgname=explicator
pkgver=0.7.0
pkgrel=1

pkgdesc="String translation library using a combination of string similarity metrics."
url="http://www.halclark.ca"
arch=('x86_64' 'i686')
license=('GPL' 'GFDL')
depends=()
makedepends=('cmake')
# optdepends=()
# conflicts=()
# replaces=()
# backup=()
# install='foo.install'
# source=("git+https://.../${pkgname}-${pkgver}.tar.gz")
# md5sums=('SKIP')
# sha1sums=('SKIP')
# options=(strip staticlibs)

build() {
  cmake "${srcdir}" -DCMAKE_INSTALL_PREFIX=/usr
  make
}

package() {
  make DESTDIR="${pkgdir}" install
}

# vim:set ts=2 sw=2 et:
