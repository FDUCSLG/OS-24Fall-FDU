#define pushp(a, b) stp a, b, [sp, #-0x10]!
#define popp(a, b) ldp a, b, [sp], #0x10 

/* `exception_vector.S` send all traps here. */
.global trap_entry
trap_entry:
// TODO: save UserContext

bl trap_global_handler

.global trap_return
trap_return:
// TODO: restore UserContext

eret
