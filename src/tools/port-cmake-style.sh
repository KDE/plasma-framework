for FS in `find $PWD -type f -name 'CMakeLists.txt' -o -name '*.cmake'`; do
    #all commands should be lowercase
    #http://techbase.kde.org/Policies/CMake_Coding_Style#Upper.2Flower_casing
    perl -p -i -e 's/^( *)([0-9A-Z_]*)( *)\(/$1.lc($2).$3."("/ge' $FS

    #removes the predicate of endif and else in cmake
    # http://techbase.kde.org/Policies/CMake_Coding_Style#End_commands
    perl -p -i -e 's/endif( *)\(.*\)/endif\1()/g' $FS
    perl -p -i -e 's/else( *)\(.*\)/else\1()/g' $FS
done
