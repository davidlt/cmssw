### RPM external gcc 4.8.0
## INITENV +PATH LD_LIBRARY_PATH %i/lib64
#Source0: ftp://gcc.gnu.org/pub/gcc/snapshots/4.7.0-RC-20120302/gcc-4.7.0-RC-20120302.tar.bz2
# Use the svn repository for fetching the sources. This gives us more control while developing
# a new platform so that we can compile yet to be released versions of the compiler.
%define gccRevision 195912
%define gccBranch trunk
Source0: svn://gcc.gnu.org/svn/gcc/trunk?module=gcc-%gccBranch-%gccRevision&revision=%gccRevision&output=/gcc-%gccBranch-%gccRevision.tar.gz

%define keep_archives true

%define gmpVersion 5.1.0a
%define mpfrVersion 3.1.1 
%define mpcVersion 1.0.1
%define islVersion 0.11.1
%define cloogVersion 0.18.0
Source1: ftp://ftp.gnu.org/gnu/gmp/gmp-%{gmpVersion}.tar.bz2
Source2: http://www.mpfr.org/mpfr-%{mpfrVersion}/mpfr-%{mpfrVersion}.tar.bz2
Source3: http://www.multiprecision.org/mpc/download/mpc-%{mpcVersion}.tar.gz
Source4: ftp://gcc.gnu.org/pub/gcc/infrastructure/isl-%{islVersion}.tar.bz2
Source5: https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/asan/scripts/asan_symbolize.py
Source6: ftp://gcc.gnu.org/pub/gcc/infrastructure/cloog-%{cloogVersion}.tar.gz

%ifos linux
%define bisonVersion 2.7
%define binutilsv 2.23.1
%define elfutilsVersion 0.155
%define m4Version 1.4.16
Source7: http://ftp.gnu.org/gnu/bison/bison-%{bisonVersion}.tar.gz
Source8: http://ftp.gnu.org/gnu/binutils/binutils-%binutilsv.tar.bz2
Source9: https://fedorahosted.org/releases/e/l/elfutils/%{elfutilsVersion}/elfutils-%{elfutilsVersion}.tar.bz2
Patch1: https://fedorahosted.org/releases/e/l/elfutils/%{elfutilsVersion}/elfutils-portability.patch
Patch2: elfutils-0.155-fix-nm-snprintf
Patch3: elfutils-0.155-fix-memset-do_oper_delete
Source10: http://ftp.gnu.org/gnu/m4/m4-%m4Version.tar.gz
%endif

%prep

%setup -T -b 0 -n gcc-%gccBranch-%gccRevision

# Filter out private stuff from RPM requires headers.
cat << \EOF > %{name}-req
#!/bin/sh
%{__find_requires} $* | \
sed -e '/GLIBC_PRIVATE/d'
EOF

%global __find_requires %{_builddir}/gcc-%{gccBranch}-%{gccRevision}/%{name}-req
chmod +x %{__find_requires}

%ifos linux
# Hack needed to align sections to 4096 bytes rather than 2MB on 64bit linux
# architectures.  This is done to reduce the amount of address space wasted by
# relocating many libraries. This was done with a linker script before, but
# this approach seems to be more correct.
cat << \EOF_CONFIG_GCC >> gcc/config.gcc
# CMS patch to include gcc/config/i386/t-cms when building gcc
tm_file="$tm_file i386/cms.h"
EOF_CONFIG_GCC

cat << \EOF_CMS_H > gcc/config/i386/cms.h
#undef LINK_SPEC
#define LINK_SPEC "%{" SPEC_64 ":-m elf_x86_64} %{" SPEC_32 ":-m elf_i386} \
 %{shared:-shared} \
 %{!shared: \
   %{!static: \
     %{rdynamic:-export-dynamic} \
     %{" SPEC_32 ":%{!dynamic-linker:-dynamic-linker " GNU_USER_DYNAMIC_LINKER32 "}} \
     %{" SPEC_64 ":%{!dynamic-linker:-dynamic-linker " GNU_USER_DYNAMIC_LINKER64 "}}} \
   %{static:-static}} -z common-page-size=4096 -z max-page-size=4096"
EOF_CMS_H
%endif

# GCC prerequisites
%setup -D -T -b 1 -n gmp-5.1.0
%setup -D -T -b 2 -n mpfr-%{mpfrVersion}
%setup -D -T -b 3 -n mpc-%{mpcVersion}
%setup -D -T -b 4 -n isl-%{islVersion}
%setup -D -T -b 6 -n cloog-%{cloogVersion}

%ifos linux
%setup -D -T -b 7 -n bison-%{bisonVersion}
%setup -D -T -b 8 -n binutils-%binutilsv
%setup -D -T -b 9 -n elfutils-%{elfutilsVersion}
%patch1 -p1
%patch2 -p1
%patch3 -p1
%setup -D -T -b 10 -n m4-%{m4Version}
%endif

%build
%ifos darwin
  CC='clang'
  CXX='clang++'
  CPP='clang -E'
  CXXCPP='clang++ -E'
  ADDITIONAL_LANGUAGES=,objc,obj-c++
  CONF_GCC_OS_SPEC=
%else
  CC=gcc
  CXX=c++
  CPP=cpp
  CXXCPP='c++ -E'
  CONF_GCC_OS_SPEC=
%endif

CC="$CC -fPIC"
CXX="$CXX -fPIC"

%ifos linux
  CONF_BINUTILS_OPTS="--enable-gold=default --enable-lto --enable-plugins --enable-threads"
  CONF_GCC_WITH_LTO="--enable-gold=yes --enable-lto" # --with-build-config=bootstrap-lto
 
  # Build M4
  cd ../m4-%{m4Version}
  ./configure --prefix=%i/tmp/m4 CC="$CC"
  make %makeprocesses
  make install
  export PATH=%i/tmp/m4/bin:$PATH
  
  # Build elfutils
  cd ../elfutils-%{elfutilsVersion}
  ./configure --disable-static --without-zlib \
             --without-bzlib --without-lzma \
             --prefix=%i CC="$CC" CXX="$CXX -Wno-strict-aliasing" CPP="$CPP" CXXCPP="$CXXCPP"
  make %makeprocesses
  make install

  # Build Bison
  cd ../bison-%{bisonVersion}
  CC="$CC" ./configure --prefix=%i/tmp/bison
  make %makeprocesses
  make install
  export PATH=%i/tmp/bison/bin:$PATH

  # Build binutils
  cd ../binutils-%{binutilsv}
  ./configure --disable-static --prefix=%i ${CONF_BINUTILS_OPTS} --disable-werror \
             CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP" CFLAGS="-I%i/include" \
             CXXFLAGS="-I%i/include" LDFLAGS="-L%i/lib"
  make %makeprocesses
  find . -name Makefile -exec perl -p -i -e 's|LN = ln|LN = cp -p|;s|ln ([^-])|cp -p $1|g' {} \; 
  make install
  which ld
%endif

# Build GMP
cd ../gmp-5.1.0
./configure --disable-static --prefix=%i --enable-shared --disable-static --enable-cxx \
 CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP"
make %makeprocesses
make install

# Build MPFR
cd ../mpfr-%{mpfrVersion}
./configure --disable-static --prefix=%i --with-gmp=%i \
 CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP"
make %makeprocesses
make install

# Build MPC
cd ../mpc-%{mpcVersion}
./configure --disable-static --prefix=%i --with-gmp=%i --with-mpfr=%i \
 CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP"
make %makeprocesses
make install

# Build ISL
cd ../isl-%{islVersion}
./configure --disable-static --with-gmp-prefix=%i --prefix=%i \
 --build=x86_64-unknown-linux-gnu --host=x86_64-unknown-linux-gnu \ 
 CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP"
make %makeprocesses
make install

# Build CLooG
cd ../cloog-%{cloogVersion}
./configure --disable-static --prefix=%i --with-gmp=system --with-gmp-prefix=%i --with-gmp-exec-prefix=%i \
 --with-isl=system --with-isl-prefix=%i --with-isl-exec-prefix=%i \
 CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP"
make %makeprocesses
make install

# Build GCC
cd ../gcc-%gccBranch-%gccRevision
mkdir -p obj
cd obj
export LD_LIBRARY_PATH=%i/lib64:%i/lib:$LD_LIBRARY_PATH
../configure --prefix=%i --disable-multilib --disable-nls \
 --enable-languages=c,c++,fortran$ADDITIONAL_LANGUAGES \
 $CONF_GCC_OS_SPEC $CONF_GCC_WITH_LTO --with-gmp=%i --with-mpfr=%i \
 --with-mpc=%i --with-isl=%i --with-cloog=%i --enable-checking=release \
 --enable-shared CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP"

make %makeprocesses bootstrap
make install

%install
cd %_builddir/gcc-%gccBranch-%gccRevision/obj && make install 

ln -s gcc %i/bin/cc
find %i/lib %i/lib64 -name '*.la' -exec rm -f {} \; || true

# Put ASan symbolizer from LLVM into bin directory
cp %SOURCE5 %i/bin/asan_symbolize.py
chmod +x %i/bin/asan_symbolize.py

# Remove unneeded documentation, temporary areas, unneeded files.
%define drop_files %i/share/{man,info,doc,locale} %i/tmp %i/lib*/{libstdc++.a,libsupc++.a}
# Strip things people will most likely never debug themself.
%define more_strip %i/bin/*{c++,g++,gcc,gfortran,gcov,cloog,cpp}*
%define strip_files %i/libexec/*/*/*/{cc1,cc1plus,f951,lto1,collect2} %i/x86_64*/bin %i/lib/lib{mpfr,gmp,cloog}* %more_strip
%define keep_archives yes
# This avoids having a dependency on the system pkg-config.
rm -rf %i/lib/pkg-config
