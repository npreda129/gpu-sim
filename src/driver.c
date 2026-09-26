int launch_threads(core_t * core, int nthreads, int binary_size, uint32_t *binary, int mem_size, uint8_t *mem) {
  // TODO validate binary and mem size
  memcpy(core->binaryMem, binary, binary_size);
  memcpy(core->sharedMem, mem, mem_size);
}
