
import * as NPRPC from './nprpc'
import * as WS from './npwebserver'
import * as SRV from './server'
import { writable, Writable } from 'svelte/store'

let webserver: WS.WebServer;
let server: SRV.Server;
let poa: NPRPC.Poa;

export let cats = new Array<WS.WebCategory>();

const VQUALITY = 0x00002000;
const IO_SPACE = 0x00004000;
const MUTABLE = 0x00008000;
const SIGNED = 0x00001000;
const INTERNAL = 0x00010000;
const SIZE_8 = 0x00000001;
const SIZE_16 = 0x00000002;
const SIZE_32 = 0x00000004;
const FLOAT_VALUE = 0x00000010;
const BIT_VALUE = 0x00000020;
const INT_VALUE = 0x00000040;
const BIT_MASK = 0x00000F00;
const SIZE_MASK = 0x0000000F;
const TYPE_MASK = BIT_VALUE | INT_VALUE | FLOAT_VALUE | SIGNED | SIZE_8 | SIZE_16 | SIZE_32;

enum VT {
	VT_UNDEFINE = (0x00000000),
	VT_DISCRETE = (SIZE_8 | BIT_VALUE),
	VT_BYTE = (SIZE_8 | INT_VALUE),
	VT_SIGNED_BYTE = (SIGNED | SIZE_8 | INT_VALUE),
	VT_WORD = (SIZE_16 | INT_VALUE),
	VT_SIGNED_WORD = (SIGNED | SIZE_16 | INT_VALUE),
	VT_DWORD = (SIZE_32 | INT_VALUE),
	VT_SIGNED_DWORD = (SIGNED | SIZE_32 | INT_VALUE),
	VT_FLOAT = (SIGNED | SIZE_32 | FLOAT_VALUE)
};

function is_vt_has_quality(type: number) {
	return type & VQUALITY ? true : false;
}

export class online_value {
	private type_: number;
	value: Writable<string>;

	private check_quality_8(data: NPRPC.Flat.Array_Direct1_u8): boolean {
		if (is_vt_has_quality(this.type_) && data.get_val(1) != 0x01) return false;
		return true;
	}

	private check_quality_16(data: NPRPC.Flat.Array_Direct1_u8): boolean {
		if (is_vt_has_quality(this.type_) && data.get_val(2) != 0x01) return false;
		return true;
	}

	private check_quality_32(data: NPRPC.Flat.Array_Direct1_u8): boolean {
		if (is_vt_has_quality(this.type_) && data.get_val(4) != 0x01) return false;
		return true;
	}

	set_value(val: SRV.Flat_server.server_value_Direct): void {
		switch (val.s) {
			case SRV.var_status.VST_UNKNOWN: this.value.set("unk"); return
			case SRV.var_status.VST_DEVICE_NOT_RESPONDED: this.value.set("nc"); return
			case SRV.var_status.VST_DEVICE_RESPONDED: 
				break;
			default:
				console.error();
		}
		let data = val.data_vd();
		const clear_type = this.type_ & TYPE_MASK;
		switch(clear_type) {
			case VT.VT_DISCRETE:
				if (!this.check_quality_8(data)) {
					this.value.set("x");
					break;
				}
				this.value.set((data.data_view.getUint8(0) === 0xFF) ? "1" : "0"); 
				break;
			case VT.VT_BYTE:
			case VT.VT_SIGNED_BYTE:
				if (!this.check_quality_8(data)) {
					this.value.set("x");
					break;
				}
				if (clear_type == VT.VT_BYTE) {
					this.value.set(data.data_view.getUint8(0).toString());
				} else {
					this.value.set(data.data_view.getInt8(0).toString());
				}
				break;
			case VT.VT_WORD:
			case VT.VT_SIGNED_WORD:
				if (!this.check_quality_16(data)) {
					this.value.set("x");
					break;
				}
				if (clear_type == VT.VT_WORD) {
					this.value.set(data.data_view.getUint16(0, true).toString());
				} else {
					this.value.set(data.data_view.getInt16(0, true).toString());
				}
				break;
			case VT.VT_DWORD:
			case VT.VT_SIGNED_DWORD:
			case VT.VT_FLOAT:
				if (!this.check_quality_32(data)) {
					this.value.set("x");
					break;
				}
				switch (clear_type) {
					case VT.VT_DWORD:
						this.value.set(data.data_view.getUint32(0, true).toString());
						break;
					case VT.VT_SIGNED_DWORD:
						this.value.set(data.data_view.getInt32(0, true).toString());
						break;
					case VT.VT_FLOAT:
						this.value.set(data.data_view.getFloat32(0, true).toFixed(3));
						break;
				}
				break;
			default:
				console.error("Unknown type %d", this.type_ & TYPE_MASK);
		}
	}

	constructor(type: number) {
		this.type_ = type;
		this.value = writable("unk");
	}
}

class OnDataCallbackImpl extends SRV._IDataCallBack_Servant {
	items_: Map<SRV.var_handle, {desc: WS.WebItem, ov: online_value}>;

	OnDataChanged(a: NPRPC.Flat.Vector_Direct2<SRV.Flat_server.server_value_Direct>): void {
		for (let x of a) {
			this.items_.get(x.h).ov.set_value(x);
			//console.log(this.items_.get(x.h).ov.value);
		}

		//for (let i of valid_items) {
		//	console.log(i.desc.name + " - " + i.ov.value);
		//}
	}

	constructor() {
		super();
		this.items_ = new Map<SRV.var_handle, {desc: WS.WebItem, ov: online_value}>();
	}
}

export async function rpc_init() {
//	NPRPC.set_debug_level(NPRPC.DebugLevel.DebugLevel_EveryCall);
	let rpc = NPRPC.init();
	poa = rpc.create_poa(32);

	let nameserver = NPRPC.get_nameserver("192.168.1.2");

	{
		let obj = NPRPC.make_ref<NPRPC.ObjectProxy>();
		await nameserver.Resolve("npsystem_webserver", obj);

		webserver = NPRPC.narrow(obj.value, WS.WebServer);
		if (!webserver) throw "WS.WebServer narrowing failed";
	}
	{
		let obj = NPRPC.make_ref<NPRPC.ObjectProxy>();
		await nameserver.Resolve("npsystem_server", obj);

		server = NPRPC.narrow(obj.value, SRV.Server);
		if (!server) throw "SRV.Server narrowing failed";
	}

	await webserver.get_web_categories(cats);

	//console.log(cats);

	let item_manager: SRV.ItemManager = null;
	{
		let obj = NPRPC.make_ref<NPRPC.ObjectProxy>();
		await server.CreateItemManager(obj);
		item_manager = NPRPC.narrow(obj.value, SRV.ItemManager);
		if (!item_manager) throw "SRV.ItemManager narrowing failed";
		item_manager.add_ref();
	}

	let data_callback = new OnDataCallbackImpl();
	let oid = poa.activate_object(data_callback);
	await item_manager.Activate(oid);

	let valid_items = new Array<{desc: WS.WebItem, ov: online_value}>();
	for (let cat of cats) {
		for (let item of cat.items) {
			let ov = new online_value(item.type);
			(item as any).ov = ov;
			if (item.dev_addr != 0xFF) valid_items.push({desc: item, ov: ov});
		}
	}

	let ar = new Array<SRV.DataDef>(valid_items.length);
	
	for (let i = 0; i < valid_items.length; ++i) {
		ar[i] = {
			dev_addr: valid_items[i].desc.dev_addr, 
			mem_addr: valid_items[i].desc.mem_addr, 
			type: valid_items[i].desc.type
		};
	}

	let handles = new Array<bigint>();
	await item_manager.Advise(ar, handles);

	for (let i = 0; i < valid_items.length; ++i) {
		data_callback.items_.set(handles[i], valid_items[i]);
	}
}