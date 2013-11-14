for FS in `find $PWD -type f -name 'CMakeLists.txt'`; do
    #removes the predicate of endif and else in cmake
    # http://techbase.kde.org/Policies/CMake_Coding_Style#End_commands
    perl -p -i -e 's/endif( *)\(.*\)/endif\1()/g' $FS
    perl -p -i -e 's/else( *)\(.*\)/else\1()/g' $FS
done
