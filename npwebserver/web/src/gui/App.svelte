<script lang="ts">
	import Category from './Category.svelte';
	import CollapsibleSection from './CollapsibleSection.svelte';
	import {cats} from '../callback'
</script>

<main>
	{#each cats as cat}
	<CollapsibleSection headerText={cat.name} expanded={true}>
		<Category cat={cat} />
	</CollapsibleSection>
	{/each}
</main>

<style>
	main {
		text-align: center;
		padding: 1em;
		width: 800px;
		margin: auto;      /* Center it */
    max-width: 100%;   /* Make it fit window if under 500px */ 
	}

/*
	h1 {
		color: #ff3e00;
		text-transform: uppercase;
		font-size: 4em;
		font-weight: 100;
	}
*/

	@media (min-width: 640px) {
		main {
			max-width: none;
		}
	}
</style>