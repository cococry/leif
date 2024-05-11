# Contributor: Luxzi <luxzi@pm.me>

pkgname="libleif"
_pkgname="leif"
pkgver='1.0'
pkgrel=1
pkgdesc="Minimal, configurable & GPU accelerated Immediate Mode UI Library written with modern OpenGL"
arch=('x86_64')
url="https://github.com/cococry/leif"
license=('GPL')
groups=()
depends=()
makedepends=('git' 'make' 'gcc' 'glfw')
provides=('libleif')
source=("${_pkgname}::git+https://github.com/cococry/${_pkgname}.git")
sha256sums=('SKIP')

pkgver() {
    cd $_pkgname
    echo $pkgver
}

build() {
    cd $_pkgname
    make 
}

package() {
    cd $_pkgname
    sudo make install
}
