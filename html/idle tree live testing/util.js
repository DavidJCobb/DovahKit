
/*int*/ function upper_bound(list, subject, comparator) {
   const size = list.length;
   let   i    = 0;
   for(; i < size - 1; ++i) {
      let item = list[i];
      let rel;
      if (comparator)
         rel = comparator(subject, item);
      else
         rel = subject < item;
      //
      if (rel)
         return i;
   }
   return i;
}
