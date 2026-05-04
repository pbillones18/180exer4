
#!/bin/bash

PROGRAM_NAME="matrix_dist"
SOURCE_FILE="lab032-ca.c"

N_SIZE=4000
NUM_SLAVES=2
START_PORT=8081

echo "Compiling..."
gcc $SOURCE_FILE -o $PROGRAM_NAME

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

TOTAL_CORES=$(nproc)
USABLE_CORES=$((TOTAL_CORES - 1))

echo "=================================="
echo "Total cores : $TOTAL_CORES"
echo "Usable cores: $USABLE_CORES"
echo "One core left idle."
echo "=================================="

echo "Starting slaves..."

for (( i=0; i<$NUM_SLAVES; i++ ))
do
    PORT=$((START_PORT + i))

    CORE=$((i % USABLE_CORES))

    echo "Slave $i -> Port $PORT -> Core $CORE"

    ./$PROGRAM_NAME \
        $N_SIZE \
        $PORT \
        1 \
        $i &

    SLAVE_PIDS[$i]=$!
done

sleep 2

echo ""
echo "Starting master..."
echo ""

./$PROGRAM_NAME $N_SIZE 0 0

echo ""
echo "Cleaning up slaves..."

for pid in "${SLAVE_PIDS[@]}"
do
    kill $pid 2>/dev/null
done

echo "Done."