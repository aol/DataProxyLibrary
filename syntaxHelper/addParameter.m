function kvp = addParameter( kvp, name, value )
	current = length(kvp) + 1;
	kvp(current).name = name;
	kvp(current).value = value;
end %addParameter
