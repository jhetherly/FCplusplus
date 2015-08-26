#ifdef FCPP_THIS_IS_NEVER_DEFINED
echo '#include "prelude.h"'
echo '#undef FCPP_MAYBE_EXTERN'
echo '#define FCPP_MAYBE_EXTERN  '
echo '#undef FCPP_MAYBE_DEFINE'
echo '#define FCPP_MAYBE_DEFINE(x) x'
echo 'namespace fcpp {'
for FILE in *.h
do
   echo "// from $FILE"
   if [ "$FILE" == "lambda.h" ] ; then
      LAM=
   elif [ "$FILE" == "monad.h" ] ; then
      LAM=
   else
      LAM=:
   fi
   $LAM echo "#ifdef FCPP_ENABLE_LAMBDA"
   cat $FILE | grep ^FCPP_MAYBE_EXTERN
   cat $FILE | grep ^FCPP_MAYBE_DEFINE 
   $LAM echo "#endif"
done
echo '}'
exit
#endif

#include "prelude.h"
#undef FCPP_MAYBE_EXTERN
#define FCPP_MAYBE_EXTERN  
#undef FCPP_MAYBE_DEFINE
#define FCPP_MAYBE_DEFINE(x) x
namespace fcpp {
// from config.h
// from curry.h
FCPP_MAYBE_EXTERN AutoCurryType _;   // this is a legal identifier as fcpp::_
FCPP_MAYBE_EXTERN Curry1 curry1, curry; // "curry" is the same as "curry1"
FCPP_MAYBE_EXTERN Curry2 curry2;
FCPP_MAYBE_EXTERN Curry3 curry3; 
// from full.h
FCPP_MAYBE_EXTERN Const const_;   // C++ keyword, so add trailing underscore
FCPP_MAYBE_EXTERN Bind1of1 bind1of1;
FCPP_MAYBE_EXTERN Bind1of2 bind1of2;
FCPP_MAYBE_EXTERN Bind2of2 bind2of2;
FCPP_MAYBE_EXTERN Bind1and2of2 bind1and2of2;
FCPP_MAYBE_EXTERN Bind1and2and3of3 bind1and2and3of3;
FCPP_MAYBE_EXTERN Bind1and2of3 bind1and2of3;
FCPP_MAYBE_EXTERN Bind2and3of3 bind2and3of3;
FCPP_MAYBE_EXTERN Bind1and3of3 bind1and3of3;
FCPP_MAYBE_EXTERN Bind1of3 bind1of3;
FCPP_MAYBE_EXTERN Bind2of3 bind2of3;
FCPP_MAYBE_EXTERN Bind3of3 bind3of3;
// from function.h
// from lambda.h
#ifdef FCPP_ENABLE_LAMBDA
FCPP_MAYBE_EXTERN fcpp_lambda::IfLambdaoid<fcpp_lambda::IfNormal> if0;
FCPP_MAYBE_EXTERN fcpp_lambda::IfLambdaoid<fcpp_lambda::IfTrue>   if1;
FCPP_MAYBE_EXTERN fcpp_lambda::IfLambdaoid<fcpp_lambda::IfFalse>  if2;
FCPP_MAYBE_EXTERN fcpp_lambda::LetRecLambdaoid letrec;
FCPP_MAYBE_EXTERN fcpp_lambda::LetLambdaoid let;
#endif
// from list.h
FCPP_MAYBE_EXTERN Head head;
FCPP_MAYBE_EXTERN Tail tail;
FCPP_MAYBE_EXTERN Null null;
FCPP_MAYBE_EXTERN Cons cons;
FCPP_MAYBE_EXTERN Cat cat;
FCPP_MAYBE_EXTERN Delay delay;
FCPP_MAYBE_EXTERN Force force;
// from monad.h
#ifdef FCPP_ENABLE_LAMBDA
FCPP_MAYBE_EXTERN Bind bind;
FCPP_MAYBE_EXTERN Bind_ bind_;
FCPP_MAYBE_EXTERN fcpp_lambda::DoM_Lambdaoid doM;
FCPP_MAYBE_EXTERN fcpp_lambda::Guard_Lambdaoid guard;
FCPP_MAYBE_EXTERN BForce bForce;
FCPP_MAYBE_EXTERN BDelay bDelay;
FCPP_MAYBE_EXTERN BLift bLift;
FCPP_MAYBE_DEFINE(ListM::Unit ListM::unit;)
FCPP_MAYBE_DEFINE(ListM::Bind ListM::bind;)
FCPP_MAYBE_DEFINE(ListM::Zero ListM::zero;)
FCPP_MAYBE_DEFINE(MaybeM::Unit MaybeM::unit;)
FCPP_MAYBE_DEFINE(MaybeM::Bind MaybeM::bind;)
FCPP_MAYBE_DEFINE(MaybeM::Zero MaybeM::zero;)
FCPP_MAYBE_DEFINE(IdentityM::Unit IdentityM::unit;)
FCPP_MAYBE_DEFINE(IdentityM::Bind IdentityM::bind;)
FCPP_MAYBE_DEFINE(ByNeedM::Unit ByNeedM::unit;)
FCPP_MAYBE_DEFINE(ByNeedM::Bind ByNeedM::bind;)
#endif
// from operator.h
FCPP_MAYBE_EXTERN MakePair makePair;
FCPP_MAYBE_EXTERN Min min;
FCPP_MAYBE_EXTERN Max max;
FCPP_MAYBE_EXTERN Plus plus;
FCPP_MAYBE_EXTERN Minus minus;
FCPP_MAYBE_EXTERN Multiplies multiplies;
FCPP_MAYBE_EXTERN Divides divides;
FCPP_MAYBE_EXTERN Modulus modulus;
FCPP_MAYBE_EXTERN Negate negate;
FCPP_MAYBE_EXTERN Equal equal;
FCPP_MAYBE_EXTERN NotEqual notEqual;
FCPP_MAYBE_EXTERN Greater greater;
FCPP_MAYBE_EXTERN Less less;
FCPP_MAYBE_EXTERN GreaterEqual greaterEqual;
FCPP_MAYBE_EXTERN LessEqual lessEqual;
FCPP_MAYBE_EXTERN LogicalAnd logicalAnd;
FCPP_MAYBE_EXTERN LogicalOr logicalOr;
FCPP_MAYBE_EXTERN LogicalNot logicalNot;
FCPP_MAYBE_EXTERN Dereference dereference;
FCPP_MAYBE_EXTERN AddressOf addressOf;
FCPP_MAYBE_EXTERN Delete_ delete_;
FCPP_MAYBE_EXTERN OutStream outStream;
FCPP_MAYBE_EXTERN InStream inStream;
FCPP_MAYBE_EXTERN PtrToFun ptr_to_fun;
FCPP_MAYBE_EXTERN Funify funify;
FCPP_MAYBE_EXTERN Inc inc;
FCPP_MAYBE_EXTERN Always1 always1;
FCPP_MAYBE_EXTERN Never1 never1;
// from pre_lambda.h
// from prelude.h
FCPP_MAYBE_EXTERN Id id;
FCPP_MAYBE_EXTERN Compose compose;
FCPP_MAYBE_EXTERN Of of;
FCPP_MAYBE_EXTERN Compose2 compose2;
FCPP_MAYBE_EXTERN Until until;
FCPP_MAYBE_EXTERN Last last;
FCPP_MAYBE_EXTERN Init init;
FCPP_MAYBE_EXTERN Length length;
FCPP_MAYBE_EXTERN At at;
FCPP_MAYBE_EXTERN Filter filter;
FCPP_MAYBE_EXTERN Concat concat;
FCPP_MAYBE_EXTERN Foldr foldr;
FCPP_MAYBE_EXTERN Foldr1 foldr1;
FCPP_MAYBE_EXTERN Foldl foldl;
FCPP_MAYBE_EXTERN Foldl1 foldl1;
FCPP_MAYBE_EXTERN Scanr scanr;
FCPP_MAYBE_EXTERN Scanr1 scanr1;
FCPP_MAYBE_EXTERN Scanl scanl;
FCPP_MAYBE_EXTERN Scanl1 scanl1;
FCPP_MAYBE_EXTERN Iterate iterate;
FCPP_MAYBE_EXTERN Repeat repeat;
FCPP_MAYBE_EXTERN Map map;
FCPP_MAYBE_EXTERN Take take;
FCPP_MAYBE_EXTERN Drop drop;
FCPP_MAYBE_EXTERN TakeWhile takeWhile;
FCPP_MAYBE_EXTERN DropWhile dropWhile;
FCPP_MAYBE_EXTERN Replicate replicate;
FCPP_MAYBE_EXTERN Cycle cycle;
FCPP_MAYBE_EXTERN SplitAt splitAt;
FCPP_MAYBE_EXTERN Span span;
FCPP_MAYBE_EXTERN Break break_; // C++ keyword, so add trailing underscore
FCPP_MAYBE_EXTERN Flip flip;
FCPP_MAYBE_EXTERN Reverse reverse;
FCPP_MAYBE_EXTERN And and_;
FCPP_MAYBE_EXTERN Or or_;
FCPP_MAYBE_EXTERN All all;
FCPP_MAYBE_EXTERN Any any;
FCPP_MAYBE_EXTERN Elem elem;
FCPP_MAYBE_EXTERN NotElem notElem;
FCPP_MAYBE_EXTERN Sum sum;
FCPP_MAYBE_EXTERN Product product;
FCPP_MAYBE_EXTERN Minimum minimum;
FCPP_MAYBE_EXTERN Maximum maximum;
FCPP_MAYBE_EXTERN ZipWith zipWith;
FCPP_MAYBE_EXTERN Zip zip;
FCPP_MAYBE_EXTERN Fst fst;
FCPP_MAYBE_EXTERN Snd snd;
FCPP_MAYBE_EXTERN Unzip unzip;
FCPP_MAYBE_EXTERN Gcd gcd;
FCPP_MAYBE_EXTERN Odd odd;
FCPP_MAYBE_EXTERN Even even;
FCPP_MAYBE_EXTERN EnumFrom enumFrom;
FCPP_MAYBE_EXTERN EnumFromTo enumFromTo;
FCPP_MAYBE_EXTERN ListUntil listUntil;
FCPP_MAYBE_EXTERN AUniqueTypeForNothing NOTHING;
FCPP_MAYBE_EXTERN Just just;
FCPP_MAYBE_EXTERN Empty empty;
FCPP_MAYBE_EXTERN HCurry hCurry;
FCPP_MAYBE_EXTERN HUncurry hUncurry;
FCPP_MAYBE_EXTERN NoOp noOp;
FCPP_MAYBE_EXTERN Before before;
FCPP_MAYBE_EXTERN After after;
FCPP_MAYBE_EXTERN Emptify emptify;
FCPP_MAYBE_EXTERN ThunkFuncToFunc thunkFuncToFunc;
FCPP_MAYBE_EXTERN Uncurry uncurry;
FCPP_MAYBE_EXTERN Uncurry0 uncurry0;
FCPP_MAYBE_EXTERN Duplicate duplicate;
FCPP_MAYBE_EXTERN Ignore ignore;
// from ref_count.h
// from reuse.h
FCPP_MAYBE_EXTERN AUniqueTypeForNil NIL;
// from signature.h
// from smart.h
}
