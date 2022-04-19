namespace cpp services
namespace java thrift.generated

struct tBlock_pos {
    1: i32 path;
    2: i32 bu;
    3: i32 offset;
}

struct tMetaRangeRet_p1 {
  1: list<byte> tag1,
  2: list<byte> tag2,
  3: list<byte> k_part1,
}

struct tMetaRangeRet_p2 {
  1: list<byte> tag2,
  2: list<byte> k_part2,
  3: i32 s,
  4: i32 oramIndex,
}

struct tPhysicalLocation {
  1: i32 leafLabel,
  2: i32 bucketLevel,
  3: i32 bucketOffset,
}

struct tMetaOblivEvict {
  1: list<byte> tag,
  2: list<i32> key_arr,
  3: i32 round,
  4: list<byte> ks,
  5: list<byte> k_part,
  6: i32 oramIndex,
  7: i32 bu,
  8: i32 path,
  9: i32 offset,
}

struct tMetaPerRe {
  1: i32 originIndex,
  2: i32 newIndex,
  3: list<byte> k_part,
}

struct tMetaPriRangeRet {
    1: list<byte> tag,
    2: list<i32> key_arr,
    3: i32 round,
    4: list<byte> ks,
    5: list<byte> k_part,
    6: i32 oramIndex,
    7: i32 path,
    8: i32 bu_from,
    9: i32 bu_max_inclusive,
    10: i32 offset,
    11: i32 stash_index,
}

struct tFullBlockContent {
  1: i32 leafLabel,
  2: i32 bucketLevel,
  3: i32 bucketOffset,
  4: list<byte> encData,
}

service ComputingEngine {
    oneway void obliv_range_ret(1: list<tBlock_pos> block_list, 2: list<tMetaRangeRet_p1> token_list_1, 3: list<tMetaRangeRet_p2> token_list_2, 4: i32 range_index, 5: i32 batch_size),
    oneway void obliv_range_evict(1: list<tMetaOblivEvict> evict_list_1, 2: list<tMetaOblivEvict> evict_list_2, 3: i32 range_index, 4: i32 batch_size),
    oneway void obliv_permute_re(1: list<tMetaPerRe> per_re_list_1, 2: list<tMetaPerRe> per_re_list_2, 3: i32 range_index, 4: i32 stash_size),
    oneway void pri_range_retrieve(1: list<tMetaPriRangeRet> retrieve_list_1, 2: list<tMetaPriRangeRet> retrieve_list_2, 3: i32 range_index, 4: i32 starting_index, 5: i32 range_size)
}

service EdgeServer {
    oneway void send_data_message (1: string data_message),

    i32 setup_rORAM_Stash (1: i32 rORAMIndex, 2: i32 stashSize, 3: i32 rangeSupport),

    i32 stash_upload_by_oram_index(1: i32 rORAMIndex, 2:list<tFullBlockContent> block_content_list, 3: i32 blocknum),

    list<tFullBlockContent> stash_fetch_by_locations (1: i32 rORAMIndex, 2: list<tPhysicalLocation> location_list, 3: i32 blocknum),

    list<tFullBlockContent> edge_fetch_by_locations (1: i32 rORAMIndex, 2: list<tPhysicalLocation> location_list, 3: i32 startLeaf, 4: i32 blocknum),

    i32 edge_upload_by_locations (1: i32 rORAMIndex, 2: list<tFullBlockContent> block_content_list, 3: i32 startLeaf, 4: i32 blocknum),

}