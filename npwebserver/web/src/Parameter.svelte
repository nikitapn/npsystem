<script lang="ts">
	import type { Writable } from 'svelte/store'

	export let name: string;
	export let desc: string;
	export let value: Writable<string>;
</script>

<li>
	<div class="name">{name}</div>
	<div class="desc">{desc}</div>
	<div class="value">{$value}</div>
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