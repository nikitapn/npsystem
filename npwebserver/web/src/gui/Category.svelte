<script lang="ts">
	import Parameter from './Parameter.svelte';
	import type { WebCategory } from '../npwebserver'
	export let cat: WebCategory;
</script>

<ul>
	{#each cat.items as param}
		<Parameter name={param.name} desc={param.description} op={param.ov.value} />
	{/each}
</ul>