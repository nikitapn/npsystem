
<script lang="ts">
	import { beforeUpdate, afterUpdate } from 'svelte';

	export let size: number;
	export let min_val: number;
	export let max_val: number;
	export let value: number;

	export let alarm_hi: number;
	export let alarm_hihi: number;

	const angle_dead_zone = Math.PI / 3;
	const start_angle = 1.5 * Math.PI - angle_dead_zone / 2.0;
	const end_angle = 1.5 * Math.PI + angle_dead_zone / 2.0;
	const angle_range = 2 * Math.PI - angle_dead_zone;
	const scale_n = 6;
	const scale_dfi = angle_range / (scale_n - 1);

	const R = size / 2 - 5;

	const to_angle = (x: number): number => {
		return start_angle - ((x - min_val) / (max_val - min_val)) * angle_range;
	}

	const to_cartesian = (fi: number, r: number) => {
		return [size/2 + r * Math.cos(fi), size/2 - r * Math.sin(fi)];
	}

	const dl000 = 0.005 * size;
	const dl00 = 0.05 * size;
	const dl0 = 0.066 * size;
	const dl1 = 0.083 * size;
	const dl2 = 0.133 * size;

	const line_width = 0.0066 * size;
	const border_width = 0.01 * size;
	const scale_width = 0.0133 * size;
	const knob_radius = 0.0166 * size;

	const scale = new Array(scale_n).fill(0).map((x: number, ix: number) => {
		let fi = start_angle - scale_dfi * ix;
		return [
			to_cartesian(fi, R - dl1), 
			to_cartesian(fi, R - dl000),
			to_cartesian(fi, R - dl2), 
			(min_val + (max_val - min_val) * ix / (scale_n-1)).toFixed(0)
		];
	});

	
	const alarm_angle0 = to_angle(alarm_hi)
	const alarm_angle1 = to_angle(alarm_hihi)


	const begin_coords = [to_cartesian(start_angle, R - dl0), to_cartesian(start_angle, R - dl000)];
	const alarm_hi_coords0 = [to_cartesian(alarm_angle0, R - dl0), to_cartesian(alarm_angle0, R - dl000)];
	const alarm_hihi_coords0 = [to_cartesian(alarm_angle1, R - dl0), to_cartesian(alarm_angle1, R - dl000)];
	const alarm_hihi_coords1 = [to_cartesian(end_angle, R - dl0), to_cartesian(end_angle, R - dl000)];

	let line_x1: number;
	let line_y1: number;

	beforeUpdate(() => {
		[line_x1, line_y1] = to_cartesian(to_angle(value), size / 2 - dl00);
	});

</script>

<div>
	<svg width={size} height={size}>
		<defs>
			<radialGradient id="radial_gradient">
				<stop offset="15%" stop-color="LightGrey" />
				<stop offset="100%" stop-color="LightSkyBlue" />
			</radialGradient>
		</defs>
		<circle cx={size/2} cy={size/2} r={R} stroke="black" stroke-width={border_width} fill="url(#radial_gradient)" />
		<circle cx={size/2} cy={size/2} r={knob_radius} fill="black" />
		
		<g stroke="none" fill="springgreen">
			<path d="
			M {begin_coords[0][0]} {begin_coords[0][1]}
			A {R-dl0} {R-dl0} 0 0 1 {alarm_hi_coords0[0][0]} {alarm_hi_coords0[0][1]}
			L {alarm_hi_coords0[1][0]} {alarm_hi_coords0[1][1]}
			A {R-dl1} {R-dl1} 0 0 0 {begin_coords[1][0]} {begin_coords[1][1]}
			L {begin_coords[0][0]} {begin_coords[0][1]}
			Z"/> 
		</g>
			
		<g stroke="none" fill="#F5F82F">
			<path d="
			M {alarm_hi_coords0[0][0]} {alarm_hi_coords0[0][1]}
			A {R-dl0} {R-dl0} 0 0 1 {alarm_hihi_coords0[0][0]} {alarm_hihi_coords0[0][1]}
			L {alarm_hihi_coords0[1][0]} {alarm_hihi_coords0[1][1]}
			A {R} {R} 0 0 0 {alarm_hi_coords0[1][0]} {alarm_hi_coords0[1][1]}
			L {alarm_hi_coords0[0][0]} {alarm_hi_coords0[0][1]}
			Z"/> 
		</g>

			<g stroke="none" fill="#F8322F">
				<path d="
				M {alarm_hihi_coords0[0][0]} {alarm_hihi_coords0[0][1]}
				A {R-dl0} {R-dl0} 0 0 1 {alarm_hihi_coords1[0][0]} {alarm_hihi_coords1[0][1]}
				L {alarm_hihi_coords1[1][0]} {alarm_hihi_coords1[1][1]}
				A {R} {R} 0 0 0 {alarm_hihi_coords0[1][0]} {alarm_hihi_coords0[1][1]}
				L {alarm_hihi_coords0[0][0]} {alarm_hihi_coords0[0][1]}
				Z"/> 
			</g>
		
		{#each scale as s}
			<line x1={s[0][0]} y1={s[0][1]} x2={s[1][0]} y2={s[1][1]} stroke="DarkSlateGrey" stroke-width={scale_width} />
			<text text-anchor="middle"  class="scale_text" x={s[2][0]} y={s[2][1]}>{s[3]}</text>
		{/each}

		<line x1={size/2} y1={size/2} x2={line_x1} y2={line_y1} stroke-width={line_width} stroke="black" />
	</svg>

</div> 

<style>
.scale_text { font: italic 10px sans-serif; }
</style>