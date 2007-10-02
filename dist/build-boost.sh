

#!/bin/sh


BLIB=false
BDEV=false
PREFIX=
HELP=false
DIST=
UNKNOWN=
REBUILD=true
TEST=false


function basename()
{
    local S=$IFS
    IFS="/"

    local bname=""
    local fname=""

    for f in $1 ; do
      if [ -z "$f" ]; then
	 continue
      fi
	
      if [ -z "$fname" ]; then
	  fname="$f"
          continue
      fi

      bname="$fname"
      fname="$fname/$f"
    done

    echo "$bname"
    IFS=$S;
}


function pkg_name()
{
    
    local my=$(echo $1 | sed -e's/\(.\+\)_.*_.*_.*/\1/')
    echo $my
}

function pkg_version()
{
    local my=$(echo $1 | sed -e 's/.\+_\([0-9]\+_[0-9]\+_[0-9]\+\)\..*/\1/')
    my=$(echo $my | sed -e 'y/_/./')
    echo $my
}

function ziptype()
{
    local my=$(echo $1 | sed -e 's/.\+\.\(.*\)/\1/')
    echo $my
}

function use()
{
   echo " build-boost [-nobuild] [-test] -dev|-lib [-prefix myprefix] boost_X_Y_Z.tar.[bz|bz2|gz|tgz]"
   exit 1
}

while [ -n "$(echo $1 | grep  '^-')" ]; do
    case $1 in
    -lib     ) BLIB=true   ;;
    -dev     ) BDEV=true   ;;
    -prefix  ) PREFIX=$(echo $2) 
	       shift       ;;
    -test    ) TEST=true   ;;
    -nobuild ) REBUILD=false ;;
    -help    ) HELP=TRUE  ;;
    *        ) UNKNOWN=$1
	       echo "UNKNOWN: $1" 
	       exit 1
	       ;;
    esac
    shift
done

DIST=$(echo $1)
NAME=$(pkg_name $DIST)
VERSION=$(pkg_version $DIST)
ZIPTYPE=$(ziptype $DIST)
TODIR=$(pwd)/$NAME-$VERSION-build
PKG=
TOP=$(pwd)

echo "BLIB:    $BLIB"
echo "BDEV:    $BDEV"
echo "PREFIX:  $PREFIX"
echo "DIST:    $DIST" 
echo "NAME:    $NAME"
echo "VERSION: $VERSION"
echo "ZIPTYPE: $ZIPTYPE"
echo "TODIR:   $TODIR"
echo "TOP:     $TOP"

if [ -z "$DIST" ]; then
    use
fi

if [ ! -f $DIST ]; then
    echo "File <$DIST> dont exist!"
    exit 1
fi

echo "Building: $DIST"

if [ "$BLIB" = "true" ]; then
    echo "A <lib> package!"
    PKG=lib
elif [ "$BDEV" = "true" ]; then 
    echo "A <dev> package!"
    PKG=dev
else
    use
fi


if [ "$PKG" = "lib" ]; then
    echo "Generating a LIB pkg!"
elif [ "$PKG" = "dev" ]; then
    echo "Generating a DEV pkg!"
fi

if [ "$TEST" = "true" ]; then
    exit 0;
fi

rm -rf $TODIR

if [ $REBUILD = "true" ]; then
    rm -rf $NAME-$VERSION

    if [ $ZIPTYPE = "bz" -o $ZIPTYPE = "bz2" ]; then
	tar jxpf $DIST
    elif [ $ZIPTYPE = "gz" -o $ZIPTYPE = "tgz" ]; then
	tar zxpf $DIST
    else
	echo "Unknown package format."
	echo "Expecting bz or gz!"
	use
    fi
fi

DIR=$(echo $VERSION | sed -e 'y/./_/')
DIR="${NAME}_$DIR"
INSTALLDIR=$TODIR/$NAME-$PKG-$VERSION

cd $DIR

if [ "$?" -ne 0 ]; then
    "Cant change directory to: $DIR"
    exit 1
fi
    

bjam --prefix=$INSTALLDIR "-sTOOLS=gcc"  install

 
cd $INSTALLDIR/lib

if [ "$?" -ne 0 ]; then
    "Cant change directory to: $INSTALLDIR/lib"
    exit 1
fi

TOLINK_SO="*-gcc-mt.so"
TOLINK_A="*-gcc-mt.a"
for f in $TOLINK_SO ; do
    LINK=$(echo $f | sed -e 's/\(.*\)-gcc-mt.so/\1/')
    echo "LINK $LINK.so -> $f"
    ln -s $f $LINK.so
done

for f in $TOLINK_A ; do
    LINK=$(echo $f | sed -e 's/\(.*\)-gcc-mt.a/\1/')
    echo "LINK $LINK.a -> $f"
    ln -s $f $LINK.a
done


cd $INSTALLDIR/include

if [ "$?" -ne 0 ]; then
    "Cant change directory to: $INSTALLDIR/include"
    exit 1
fi

TOLINK="boost-*"

for f in $TOLINK ; do
    echo "LINK boost -> $f/boost"
    ln -s $f/boost boost
    break;
done



echo "$NAME-$PKG" > $INSTALLDIR/NAME
echo "$VERSION"   > $INSTALLDIR/VERSION


(cd $INSTALLDIR; tar xpf $TOP/mipkg.tar)


if [ "$PKG" = "lib" ]; then
    rm -rf $INSTALLDIR/include
    rm -f  $INSTALLDIR/lib/libboost*.a
    rm -f  $INSTALLDIR/lib/libboost*-gcc-mt-d-*
    rm -f  $INSTALLDIR/lib/libboost*-gcc-d-*
    rm -f  $INSTALLDIR/lib/libboost*-gcc-d.*
    echo "lib/libboost*"      >  $INSTALLDIR/mipkg/files
elif [ "$PKG" = "dev" ]; then
#    rm -f  $INSTALLDIR/lib/libboost*.a
    rm -f  $INSTALLDIR/lib/libboost*-gcc-mt-d-*
    rm -f  $INSTALLDIR/lib/libboost*-gcc-d-*
    rm -f  $INSTALLDIR/lib/libboost*-gcc-d.*
    echo "lib/libboost*"  >  $INSTALLDIR/mipkg/files
    echo "include/boost" >>  $INSTALLDIR/mipkg/files
else
    echo "Which package shall be generated LIB or DEV!"
    use
fi


cd $TODIR

if [ "$?" -ne 0 ]; then
    echo "Cant change directory to <$TODIR>!"
    exit 1
fi

tar cpf $NAME-$PKG-$VERSION.tar $NAME-$PKG-$VERSION
gzip $NAME-$PKG-$VERSION.tar

mv $NAME-$PKG-$VERSION.tar.gz $TOP

echo "Ny pakke  $NAME-$PKG-$VERSION.tar.gz"

exit 0
