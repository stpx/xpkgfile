command_not_found_handler() {
	local pkgs cmd="$1"

	pkgs=(${(f)"$(xpkgfile --search /usr/bin/"$cmd" 2>/dev/null)"})
	if [[ -n "$pkgs" ]]; then
		printf '%s may be found in the following packages:\n' "$cmd"
		printf ' %s\n' $pkgs[@]
		return 0
	fi

	return 127
}
