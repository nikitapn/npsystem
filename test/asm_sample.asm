.code

wait_for proc
wt:
	dec rcx
	jnz wt
	ret
wait_for endp
end