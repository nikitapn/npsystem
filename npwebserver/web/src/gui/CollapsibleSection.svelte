<script>
	// based on suggestions from:
	// Inclusive Components by Heydon Pickering https://inclusive-components.design/collapsible-sections/
	export let headerText;
	export let expanded = false

	import { slide } from 'svelte/transition';
	import { quintOut } from 'svelte/easing';

</script>

<div class="collapsible">
	<h3>
		<button aria-expanded={expanded} on:click={() => expanded = !expanded}>{headerText}
			<svg viewBox="0 0 20 20" fill="none" >
				<path class="vert" d="M10 1V19" stroke="black" stroke-width="2"/>
				<path d="M1 10L19 10" stroke="black" stroke-width="2"/>
			</svg>
		</button>
	</h3>
	{#if expanded}
	<div class='contents' transition:slide="{{delay: 0, duration: 250, easing: quintOut }}" >
		<slot></slot>
	</div>
	{/if}
</div>

<style>
.collapsible {
	border-bottom: 1px solid var(--gray-light, #eee);
}

h3 {
	margin: 0;
}

button {
	background-color: var(--background, #fff);
	color: var(--gray-darkest, #6ca6a8);
	display: flex;
	justify-content: space-between;
	width: 100%;
	border: none;
	margin: 0;
	padding: 1em 0.5em;
	background-color: beige;
	cursor: pointer;
}
button:hover {
	background-color: rgb(167, 241, 124);
}

button[aria-expanded="true"] {
	border-bottom: 1px solid var(--gray-light, #eee);
}

	button[aria-expanded="true"] .vert {
			display: none;
	}

	button:focus svg{
			outline: 2px solid;
	}

	button [aria-expanded="true"] rect {
			fill: currentColor;
	}

	svg {
			height: 0.7em;
			width: 0.7em;
	}
</style>