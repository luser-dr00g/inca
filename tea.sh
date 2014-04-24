# tea.sh
# attempt to re-write the math from my postscript program
# which draws the Utah Teapot.
# http://codegolf.stackexchange.com/a/25900/2381
# lack of floating-point in inca is a big hurdle.

./inca `cat ./teapot` <<END
b<;>0{a
c<((-1)@b>~#a)/a
d<;>(1+b){a
e<((-2+b)@d>~#a)/a
s<:((!(0{x)=y)\y)+((0{x)=y)\(0!1+~#y).(1{x)
h<:(y<((~(#y)-1){y))
i<:$,(((1~(0{:,)=y)>~#y)/y)
j<:$,((((1~x-(1~x<(0{:,)=y)=~#y)>~#y)-(1+1~x)>~#y)/y)
k<:$,(((1~x-(1~(x<(0{:,)=(y<'hy)))=~#y)<~#y)/y)
p<:$,y,(0=1+~<(5-(#((1~((0{:.)=y))<~#y)/y)))
q<:(1{y)+100000.0{y
v<>3{e
x<'q;$,((:. )"s'p'iv),0
y<'q;$,((:. )"s'p'jv),0
z<'q;$,((:. )"s'p'kv),0
END

#f<>0{e
#g<,((!(0{:,)=f)\f)+((0{:,)=f)\(0!1+~#f).(0{:;)
#h<,((!(0{:.)=g)\g)+((0{:.)=g)\(0!1+~#g).(0{: )
#i<;$,h
#f<>0{e
#g<$,(:,;)"sf
#h<$,(:. )"sg
#i<;$,h

#'hv
#'p'iv

#the variable a is set to the box-command-array of the argv[] strings
#     0{a fetches argv[1]
#    >0{a unboxes it
#   ;>0{a executes the string, yielding the number 32 (from the teapot dataset)
# b<;>0{a store the result as the variable b
#            #a is the length of a, or argc-1
#           ~#a is an iota vector from 0..argc-1-1
#         b>~#a yields a boolean vector, same length as a, with 1s in the first b slots
#    (-1)@b>~#a rotates the boolean vector down by 1, so 0 then b 1s then 0s filling out length of a
#   ((-1)@b>~#a)/a compress a with this boolean vector, yielding the patch data
# c<((-1)@b>~#a)/a store the result as the variable c
#     (1+b){a fetch argv[b+1], the argv[] element with the number of vertices
#    >(1+b){a unbox it
#   ;>(1+b){a execute the string, yielding the number 306
# d<;>(1+b){a store the result as the variable d
#          d>~#a yields a boolean vector, same length as a, with 1s in the first d slots
#   (-2+b)@d>~#a rotate down by b+2, so b+2 0s, then d 1s
#  ((-2+b)@d>~#a)/a compress a with this boolean vector, yielding the vertex data
#   f<>0{e   f is the unboxed command-string of the first vertex
#       1.4,0.0,2.4
#       (0{:,)=f yields a boolean vector the length of f, with 1s where the commas are in f
#     (!(0{:,)=f) boolean vector with 1s where f is not a comma
#    ((!(0{:,)=f)\f) expand f by this vector, yielding f with 0s where the commas were
#      ((0{:,)=f) boolean vector length of f, with 1s where the commas are in f (again)
#       (0!1+~#f) vector of 1s length of f
#       (0!1+~#f).(0{:;) vector of semicolons ;;;;;;;; length of f
#     ((0{:,)=f)\(0!1+~#f).(0{:;)  vector with semicolons where the commas are in f and 0s elsewhere
#  ((!(0{:,)=f)\f) + ((0{:,)=f)\(0!1+~#f).(0{:;)  yield f with commas replaced by semicolons
# g<, ravel and store result as variable g
#  ((!(0{:.)=g)\g)+((0{:.)=g)\(0!1+~#g).(0{: )  yield g with periods replaced by spaces
# h<, ravel and store result as variable h
#  i<;$,h execute h and store result in i, yielding
#    1 4 
#    0 0 
#    2 4 
#   repeating the process for vertex 1, yields incorrect results, since the negative
#   function in the y coordinate extends to the z coordinate as well.

# substitution function s:
# s<:((!(0{x)=y)\y)+((0{x)=y)\(0!1+~#y).(1{x)
# replaces occurrences of 0{x in y with 1{x
# g<$,(:,;)"sf

