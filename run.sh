bin=$1
if [ -z $1 ]
  then
    bin="ServerStatus"
fi

./build.sh
if [ $? -eq 0 ];
then
  (cd ./build/bin && clear && ./$bin)
fi