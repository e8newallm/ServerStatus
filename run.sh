bin=$1
if [ -z $1 ]
  then
    bin="serverstatus"
fi

./build.sh
if [ $? -eq 0 ];
then
  (cd ./build/bin && clear && ./$bin)
fi