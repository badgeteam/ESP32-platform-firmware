import sys, traceback
sys.path.append('esp-idf/components/partition_table')

from gen_esp32part import PartitionTable


################################################
## Prints partition offset or size to STDOUT
################################################

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print('Usage: dump_partitions <path/to/partitions.csv> <partition_name> <offset|size>')
        exit(-1)

    filename = sys.argv[1]
    partition_name = sys.argv[2]
    information_type = sys.argv[3]  # should be either 'offset' or 'size'

    with open(filename, 'r') as partition_file:
        table = PartitionTable.from_csv(partition_file.read())

    try:
        partition = table[partition_name]
        print('0x%02x' % getattr(partition, information_type))
    except BaseException as err:
        print('Couldn\'t read partition information: %s' % err)
        print(stacktrace.format_exc())
        exit(-1)