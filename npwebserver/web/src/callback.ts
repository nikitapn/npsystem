
import * as NPRPC from './nprpc'
import * as WS from './npwebserver'
import * as SRV from './server'
import OnlineValue from './online_value'

let webserver: WS.WebServer;
let server: SRV.Server;
let poa: NPRPC.Poa;

export let cats = new Array<WS.WebCategory>();

export async function rpc_init() {
	//	NPRPC.set_debug_level(NPRPC.DebugLevel.DebugLevel_EveryCall);
	let rpc = await NPRPC.init();
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

	let valid_items = new Array<{ desc: WS.WebItem, ov: OnlineValue }>();
	for (let cat of cats) {
		for (let item of cat.items) {
			let ov = new OnlineValue(item.type);
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

class OnDataCallbackImpl extends SRV._IDataCallBack_Servant {
	items_: Map<SRV.var_handle, { desc: WS.WebItem, ov: OnlineValue }>;

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
		this.items_ = new Map<SRV.var_handle, { desc: WS.WebItem, ov: OnlineValue }>();
	}
}