// Copyright (c) 2022 nikitapnn1@gmail.com
// This file is a part of Nikita's NPK calculator and covered by LICENSING file in the topmost directory

import * as NPRPC from 'nprpc';
import * as benchmark from 'rpc/benchmark'

export let poa: NPRPC.Poa;
export let benchmark_server: benchmark.Benchmark;

export const init_rpc = async (): Promise<void> => {
	NPRPC.set_debug_level(NPRPC.DebugLevel.DebugLevel_Critical);
	let rpc = await NPRPC.init();
	poa = rpc.create_poa(10);
	benchmark_server = NPRPC.narrow(rpc.host_info.objects.benchmark, benchmark.Benchmark);
}