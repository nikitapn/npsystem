<script lang="ts">
	import type { Writable } from 'svelte/store'
	import type {OpVal} from '../online_value'
	import Gauge from './Gauge.svelte'

	export let name: string;
	export let desc: string;
	export let op: Writable<OpVal>;
	//
</script>

<li>
	<div class="name">{name}</div>
	<div class="desc">{desc}</div>
	<div class="value">{$op.str}</div>
	<Gauge size={200} min_val={0} max_val={100.0} value={$op.value} alarm_hi={60} alarm_hihi={90} />
</li>

<style>
	li { 
		display: flex; 
		background-color: rgba(127, 219, 228, 0.342);
		margin: 5px;
	}
	
	.name {
		text-align: left;
		padding: 1em;
		width: 220px;
		max-width: 220px;
	}

	.desc {
		text-align: left;
		padding: 1em;
		width: 260px;
		max-width: 260px;
	}

	.value {
		text-align: right;
		padding: 1em;
	}
	
</style>