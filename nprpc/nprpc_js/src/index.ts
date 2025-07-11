// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

export { Exception } from './base'
export * from './nprpc'
export * from './utils'
export * as Flat from './flat'
export { FlatBuffer } from './flat_buffer'
export * from './gen/nprpc_base'
export * from './gen/nprpc_nameserver'

import { Nameserver, _INameserver_Servant } from './gen/nprpc_nameserver'
import { detail } from './gen/nprpc_base';

export function get_nameserver(nameserver_ip: string): Nameserver {
	const oid: detail.ObjectId = {
		object_id: 0n,
		poa_idx: 0,
		flags: detail.ObjectFlag.Persistent,
		origin: new Array<number>(16).fill(0),
		class_id: _INameserver_Servant._get_class(),
		urls: "ws://" + nameserver_ip + ":15001",
	};
	const nameserver = new Nameserver(oid);
	nameserver.select_endpoint();
	return nameserver;
}