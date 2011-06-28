function value = getParameter( kvp, name )
value = [];

for i = 1 : length( kvp )
    if strcmpi( name, kvp(i).name )
        value = kvp(i).value;
        return;
    end
end
