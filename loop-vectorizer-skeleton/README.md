## LLVM Pass Skeleton

Simple example of LLVM pass from summer school. This was compiled/run with clang/llvm trunk installation.

### Build

Once you have Clang/LLVM trunk installed, you can build this pass as:

```bash
export LLVM_INSTALL_PATH=$HOME/workarena/repos/ext/paris-clang-llvm-2017/pramodskumbhar/Scripts/install/debug/trunk:$PATH
export PATH=$LLVM_INSTALL_PATH/bin:$PATH

cd loop-vectorizer-skeleton
mkdir -p build && cd build
cmake .. -DCMAKE_C_COMPILER=`which clang` -DCMAKE_CXX_COMPILER=`which clang++` -DCMAKE_BUILD_TYPE=Debug -DLLVM_CONFIG=$LLVM_INSTALL_PATH/bin/llvm-config -DCMAKE_CXX_FLAGS="-std=c++11"
make VERBOSE=1
```
### Loop Canonicalization

First generate LLVM IR file from test code. Note that we have to use higher optimization flags (e.g. `-O3`) in order to generate metadata information which is required during optimization passes. If we don't do this then optimization passes doesn't run.

```bash
 clang  -emit-llvm -O3 -mllvm -disable-llvm-optzns -S -c ../test/vector-addition.cpp -o vector-addition
```

This will produce the loop which will look like:

```bash
$ cat vector-addition

define void @_Z15vector_additionPfS_(float* %A, float* %B) #0 {
entry:
  ...
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i64, i64* %i, align 8, !tbaa !7
  %cmp = icmp slt i64 %1, 1024
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast i64* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %2) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  ...
  store float %add, float* %arrayidx1, align 4, !tbaa !9
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i64, i64* %i, align 8, !tbaa !7
  %inc = add nsw i64 %9, 1
  store i64 %inc, i64* %i, align 8, !tbaa !7
  br label %for.cond
```

The above loop is not in canonical form. (see unconditional branch from `entry` block to `for.cond`). We can apply some loop transformation passes to turn it into canonical form. This can be achieved by:

```bash
$ opt -load -mem2reg -instnamer -loop-rotate  -S  vector-addition

entry:
  ...
  %cmp11 = icmp slt i64 %tmp110, 1024
  br i1 %cmp11, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge, %entry
  %tmp2 = bitcast i64* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %tmp2) #2
  br label %for.end

for.body:
  ...                                             ; preds = %for.body.lr.ph, %for.inc
  br label %for.inc

for.inc:
  ...                                             ; preds = %for.body
  br i1 %cmp, label %for.body, label %for.cond.for.cond.cleanup_crit_edge
```

Notice the unconditional branch is now replaced by conditional.

### Running A Pass

Now we can run the pass using `opt` tool. Build the code as :

```bash
cd build
cmake .. -DCMAKE_C_COMPILER=`which clang` -DCMAKE_CXX_COMPILER=`which clang++` -DCMAKE_BUILD_TYPE=Debug -DLLVM_CONFIG=$LLVM_INSTALL_PATH/bin/llvm-config -DCMAKE_CXX_FLAGS="-std=c++11"
make VERBOSE=1
```

And run pass as:

```bash
opt -load simple-vectorizer/libSimpleVectorizerPass.so -mem2reg -instnamer -loop-rotate -simple-vectorizer vector-addition
```

If you want to print / dump IR then use `-S` option:

```bash
opt -load simple-vectorizer/libSimpleVectorizerPass.so -mem2reg -instnamer -loop-rotate  -simple-vectorizer  -S  vector-addition
```

When you want to check/debug the passes being run, use `debug-pass` option as:

```bash
opt -load simple-vectorizer/libSimpleVectorizerPass.so -mem2reg -instnamer -loop-rotate  -simple-vectorizer  -S  vector-addition -debug-pass=Executions
```
