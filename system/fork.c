/* fork.c - fork */

#include <xinu.h>

pid32 fork() 
{   
    uint32      ssize;
    struct  procent *pptr;
    uint32      savsp, *pushsp;
    intmask     mask;        /* Interrupt mask        */
    pid32       pid;         /* Stores new process id */
    struct  procent *cptr;   /* Pointer to proc. table entry */
    uint32      *saddr;      /* Stack address         */

    mask = disable();

    pptr = &proctab[currpid];
    ssize = pptr->prstklen; /* Copy parent's stack lenth  */
    if (ssize < MINSTK)
        ssize = MINSTK;
    ssize = (uint32) roundmb(ssize);
    if (((pid=newpid()) == SYSERR) ||
        ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR)) {
        restore(mask);
        return SYSERR;
    }

    prcount++;
    cptr = &proctab[pid];

    /* Initialize process table entry for new process */
    cptr->prstate    = PR_SUSP;       /* Initial state is suspended */
    cptr->prprio     = pptr->prprio;    /* Copy parent's priority */
    cptr->prstkbase  = (char *)saddr;
    cptr->prstklen   = ssize;
    strncpy(cptr->prname, pptr->prname, PNMLEN);    /* Copy parent's process name */
    cptr->prsem      = -1;
    cptr->prparent   = currpid;
    cptr->prhasmsg   = FALSE;
    cptr->user_process = pptr->user_process;

    /* Set up stdin, stdout, and stderr descriptors */
    cptr->prdesc[0] = CONSOLE;
    cptr->prdesc[1] = CONSOLE;
    cptr->prdesc[2] = CONSOLE;

    /* ------- Build the child's stack by copying the parent stack (top-down) ------- */
    uint32 * ebp;
    asm("movl %%ebp, %0" : "=r"(ebp));
    uint32 * parent_currptr;    // Scan pointer over parent's stack (from high to low)
    uint32 * frame_ptr;         // Value used to detect "saved EBP" cells during copy
    parent_currptr = (uint32 *) pptr->prstkbase;   // Start at parent's stack high address
    frame_ptr = (uint32 *) pptr->prstkbase; // First "frame marker" is the parent's high address
    savsp = (uint32) saddr; // Child-side frame pointer seed (starts at child stack high)

    /*
     * Copy parent's stack words down to (but not including) the current EBP region.
     * When a copied word equals 'frame_ptr', treat it as a saved-EBP field and
     * write the child-side frame pointer (savsp) instead; then update savsp and
     * move the 'frame_ptr' to the just-matched parent address to continue chaining.
     */

    while((uint32)parent_currptr > (uint32)ebp) {
        if (*parent_currptr == (uint32)frame_ptr) {
            *saddr = savsp;             // Rewrite saved EBP with child-side frame link
            savsp = (uint32) saddr;     // Advance child-side frame pointer
            frame_ptr = parent_currptr; // Next match target is this parent address
            --saddr;                    // Move child write cursor down
            --parent_currptr;           // Move parent read cursor down
        } else {
            *saddr = *parent_currptr;   // Normal word: straight copy
            --saddr;
            --parent_currptr;
        }
    }

    /*
     * Close the current frame link on the child side:
     * write the (child-side) saved-EBP for the frame at the boundary.
     */
    *saddr = savsp;

    savsp = (uint32) saddr;             // 'savsp' now marks start of the context frame

    *--saddr = 0x00000200;

    *--saddr = NPROC;     /* eax: child's fork() return value */
    *--saddr = 0;         /* ecx */
    *--saddr = 0;         /* edx */
    *--saddr = 0;         /* ebx */
    *--saddr = 0;         /* esp (placeholder); patched via pushsp below */
    pushsp = saddr;       /* remember where ESP is stored */
    *--saddr = savsp;     /* ebp */
    *--saddr = 0;         /* esi */
    *--saddr = 0;         /* edi */
    *pushsp = (unsigned long)(cptr->prstkptr = (char *)saddr);

    /* Mark child ready */
    cptr->prstate = PR_READY;
    insert(pid, readylist, cptr->prprio);

    restore(mask);
    return pid;
}