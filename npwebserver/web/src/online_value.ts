import * as NPSYSC from './npsystem_constants'
import * as SRV from './server'
import * as NPRPC from './nprpc'
import { writable, Writable } from 'svelte/store'

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


export interface OpVal {
	value: number;
	status: SRV.var_status;
	quality: NPSYSC.Quality;
	str: string;
};

export default class OnlineValue {
	private type_: number;
	public value: Writable<OpVal>;

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
			case SRV.var_status.VST_UNKNOWN:
				this.value.set({ value: null, status: val.s, quality: NPSYSC.Quality.VQ_BAD, str: "unk" });
				return;
			case SRV.var_status.VST_DEVICE_NOT_RESPONDED:
				this.value.set({ value: null, status: val.s, quality: NPSYSC.Quality.VQ_BAD, str: "nc" });
				return;
			case SRV.var_status.VST_DEVICE_RESPONDED:
				let value: number, 
					quality = NPSYSC.Quality.VQ_GOOD, 
					str: string;

				let data = val.data_vd();
				const clear_type = this.type_ & TYPE_MASK;
				
				switch (clear_type) {
					case VT.VT_DISCRETE:
						if (!this.check_quality_8(data)) {
							str = "x";
							quality = NPSYSC.Quality.VQ_BAD;
							break;
						}
						value = data.data_view.getUint8(0);
						str = (data.data_view.getUint8(0) === NPSYSC.Boolean.True) ? "1" : "0";
						break;
					case VT.VT_BYTE:
					case VT.VT_SIGNED_BYTE:
						if (!this.check_quality_8(data)) {
							str = "x";
							quality = NPSYSC.Quality.VQ_BAD;
							break;
						}
						if (clear_type == VT.VT_BYTE) {
							value = data.data_view.getUint8(0);
							str = value.toString();
						} else {
							value = data.data_view.getInt8(0);
							str = value.toString();
						}
						break;
					case VT.VT_WORD:
					case VT.VT_SIGNED_WORD:
						if (!this.check_quality_16(data)) {
							str = "x";
							quality = NPSYSC.Quality.VQ_BAD;
							break;
						}
						if (clear_type == VT.VT_WORD) {
							value = data.data_view.getUint16(0, true);
							str = value.toString();
						} else {
							value = data.data_view.getInt16(0, true);
							str = value.toString();
						}
						break;
					case VT.VT_DWORD:
					case VT.VT_SIGNED_DWORD:
					case VT.VT_FLOAT:
						if (!this.check_quality_32(data)) {
							str = "x";
							quality = NPSYSC.Quality.VQ_BAD;
							break;
						}
						switch (clear_type) {
							case VT.VT_DWORD:
								value = data.data_view.getUint32(0, true);
								str = value.toString();
								break;
							case VT.VT_SIGNED_DWORD:
								value = data.data_view.getInt32(0, true);
								str = value.toString();
								break;
							case VT.VT_FLOAT:
								value = data.data_view.getFloat32(0, true);
								str = value.toFixed(3);
								break;
						}
						break;						
					default:
						console.error("Unknown type %d", this.type_ & TYPE_MASK);
						return;
				}
				this.value.set({ value: value, status: val.s, quality: quality, str: str });
				break;
			default:
				console.error();
				return;
		}
	}

	constructor(type: number) {
		this.type_ = type;
		this.value = writable({ value: 0, status: SRV.var_status.VST_UNKNOWN, quality: 0, str: "unk" });
	}
}
