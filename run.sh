folder=./Data/cs_artifact/
q_num=10000
projects=("parser" "vpr" "crafty" "twolf" "eon" "gap" "vortex" "perlbmk" "gcc")
#projects=("gap" "vortex" "perlbmk")


for p in ${projects[@]}; do
  echo ./cfl -n $q_num -fd $folder$p/
  ./csr -n $q_num -fd $folder$p/
done