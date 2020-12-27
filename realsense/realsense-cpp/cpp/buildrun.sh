basepath=$(cd `dirname $0`; pwd)
cd $basepath
cmake .
make clean
make -j4
./RSCPP

