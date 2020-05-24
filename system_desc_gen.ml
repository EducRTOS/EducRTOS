(* let p = Printf.printf;; *)
let ps = print_string;;
let pf = Printf.printf;;
let doit n =
  ps "#include \"user_tasks.h\"                                           \n";
  ps "                                                                    \n";
  ps "#define STRING(x) #x                                                \n";
  ps "#define XSTRING(x) STRING(x)                                        \n";
  ps "                                                                    \n";
  ps "#define INCBIN(name, file)                                        \\\n";
  ps "    asm(\".section .data.task\\n\"                                \\\n";
  ps "            \".global \" XSTRING(name) \"_begin\\n\"              \\\n";
  ps "            \".type \" XSTRING(name) \"_begin, @object\\n\"       \\\n";
  ps "            \".balign 16\\n\"                                     \\\n";
  ps "            XSTRING(name) \"_begin:\\n\"                          \\\n";
  ps "            \".incbin \\\"\" file \"\\\"\\n\"                     \\\n";
  ps "                                                                  \\\n";
  ps "            \".global \" XSTRING(name) \"_end\\n\"                \\\n";
  ps "            \".type \" XSTRING(name) \"_end, @object\\n\"         \\\n";
  ps "            \".balign 1\\n\"                                      \\\n";
  ps "            XSTRING(name) \"_end:\\n\"                            \\\n";
  ps "            \".byte 0\\n\"                                        \\\n";
  ps "    );                                                            \\\n";
  ps "    extern __attribute__((aligned(16))) char name ## _begin[];    \\\n";
  ps "    extern char name ## _end[];                                   \\\n";
  ps "                                                                    \n";
  for i = 0 to n - 1 do
  pf "INCBIN(task%d, \"task0.bin\")                                       \n" i;
  done;
  ps "#include \"terminal.h\"           /* For now. */                    \n"; 
  ps "#include \"user_tasks.h\"                                           \n";
  ps "#include \"high_level.h\"                                           \n";
  ps "                                                                    \n";
  ps "                                                                    \n";
  pf "#define NB_TASKS %d                                                 \n"  n;
  ps "#include \"system_desc.h\"                                          \n";
  ps "                                                                    \n";
  ps "/* Note: we could put this description at the beginning of each task\n"; 
  ps "   It would make it easy to pass tasks on the command line. */      \n";
  ps "static const struct task_description tasks[] = {                    \n";
  for i = 0 to n - 1 do
  pf "  [%d] = {                                                          \n" i;
  pf "     .context = &system_contexts[%d],                               \n" i;
  ps "     .start_pc = 0,                                                 \n";
  pf "     .task_begin = task%d_begin,                                    \n" i;
  pf "     .task_end = task%d_end,                                        \n" i;
  ps "#ifdef FP_SCHEDULING                                                \n";
  ps "     .priority = 10,                                                \n";
  ps "#endif                                                              \n";
  ps "  },                                                                \n";
  done;
  ps "};                                                                  \n";
  ps "                                                                    \n";
  ps "                                                                    \n";
  ps "static struct context *ready_heap_array[NB_TASKS];                  \n";
  ps "static struct context *waiting_heap_array[NB_TASKS];                \n";
  ps "                                                                    \n";
  ps "const struct user_tasks_image user_tasks_image = {                  \n";
  pf "  .nb_tasks = NB_TASKS,                                             \n";
  ps "  .tasks = tasks,                                                   \n";
  ps "  .low_level = {}/* low_level_description */,                       \n";
  ps "  .ready_heap_array = &ready_heap_array[0],                         \n";
  ps "  .waiting_heap_array = &waiting_heap_array[0],                     \n";
  ps "};                                                                  \n";
;;

let num = Stdlib.int_of_string @@ Sys.argv.(1) in
doit num;;
  
