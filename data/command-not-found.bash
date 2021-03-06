command_not_found_handle() {
	local pkgs cmd=$1
	local FUNCTEST=10

	set +o verbose

	mapfile -t pkgs < <(pkgfile -s -- "$cmd" 2>/dev/null) 
	if (( ${#pkgs[*]} )); then
		printf '%s may be found in the following packages:\n' "$cmd"
		printf ' %s\n' "${pkgs[@]}"
		return 0
	else
		printf "bash: %s: command not found\n" "$cmd" >&2
		return 127
	fi
}
