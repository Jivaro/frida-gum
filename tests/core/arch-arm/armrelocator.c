/*
 * Copyright (C) 2010-2014 Ole André Vadla Ravnås <ole.andre.ravnas@tillitech.com>
 *
 * Licence: wxWindows Library Licence, Version 3.1
 */

#include "armrelocator-fixture.c"

TEST_LIST_BEGIN (armrelocator)
  RELOCATOR_TESTENTRY (one_to_one)
  RELOCATOR_TESTENTRY (b_imm_a1_positive_should_be_rewritten)
  RELOCATOR_TESTENTRY (b_imm_a1_negative_should_be_rewritten)
  RELOCATOR_TESTENTRY (bl_imm_a1_positive_should_be_rewritten)
  RELOCATOR_TESTENTRY (bl_imm_a1_negative_should_be_rewritten)
  RELOCATOR_TESTENTRY (blx_imm_a2_positive_should_be_rewritten)
  RELOCATOR_TESTENTRY (blx_imm_a2_negative_should_be_rewritten)
TEST_LIST_END ()

RELOCATOR_TESTCASE (one_to_one)
{
  const guint32 input[] = {
    GUINT32_TO_LE (0xe1a0c00d), /* mov ip, sp    */
    GUINT32_TO_LE (0xe92d0030), /* push {r4, r5} */
  };
  const GumArmInstruction * insn;

  SETUP_RELOCATOR_WITH (input);

  insn = NULL;
  g_assert_cmpuint (gum_arm_relocator_read_one (&fixture->rl, &insn), ==, 4);
  g_assert_cmpint (insn->mnemonic, ==, GUM_ARM_MOV);
  assert_outbuf_still_zeroed_from_offset (0);

  insn = NULL;
  g_assert_cmpuint (gum_arm_relocator_read_one (&fixture->rl, &insn), ==, 8);
  g_assert_cmpint (insn->mnemonic, ==, GUM_ARM_PUSH);
  assert_outbuf_still_zeroed_from_offset (0);

  g_assert (gum_arm_relocator_write_one (&fixture->rl));
  g_assert_cmpint (memcmp (fixture->output, input, 4), ==, 0);
  assert_outbuf_still_zeroed_from_offset (4);

  g_assert (gum_arm_relocator_write_one (&fixture->rl));
  g_assert_cmpint (memcmp (fixture->output + 4, input + 1, 4), ==, 0);
  assert_outbuf_still_zeroed_from_offset (8);

  g_assert (!gum_arm_relocator_write_one (&fixture->rl));
}

/*
 * Branch instruction coverage based on DDI0487A_b_armv8_arm.pdf
 *
 * B imm (F7.1.18)
 * [x] A1
 *
 * BL, BLX imm (F7.1.25)
 * [x] A1
 * [x] A2
 */

typedef struct _BranchScenario BranchScenario;

struct _BranchScenario
{
  GumArmMnemonic mnemonic;
  guint32 input[1];
  gsize input_length;
  guint32 expected_output[4];
  gsize expected_output_length;
  gsize pc_offset;
  gssize expected_pc_distance;
  gssize lr_offset;
  gssize expected_lr_distance;
};

static void branch_scenario_execute (BranchScenario * bs,
    TestArmRelocatorFixture * fixture);

RELOCATOR_TESTCASE (b_imm_a1_positive_should_be_rewritten)
{
  BranchScenario bs = {
    GUM_ARM_B_IMM_A1,
    { 0xea000001 }, 1,          /* b pc + 4          */
    {
      0xe51ff004,               /* ldr pc, [pc, #-4] */
      0xffffffff                /* <calculated PC    */
                                /*  goes here>       */
    }, 2,
    1, 4,
    -1, -1
  };
  branch_scenario_execute (&bs, fixture);
}

RELOCATOR_TESTCASE (b_imm_a1_negative_should_be_rewritten)
{
  BranchScenario bs = {
    GUM_ARM_B_IMM_A1,
    { 0xeaffffff }, 1,          /* b pc - 4          */
    {
      0xe51ff004,               /* ldr pc, [pc, #-4] */
      0xffffffff                /* <calculated PC    */
                                /*  goes here>       */
    }, 2,
    1, -4,
    -1, -1
  };
  branch_scenario_execute (&bs, fixture);
}

RELOCATOR_TESTCASE (bl_imm_a1_positive_should_be_rewritten)
{
  BranchScenario bs = {
    GUM_ARM_BL_IMM_A1,
    { 0xeb000001 }, 1,          /* bl pc + 4         */
    {
      0xe59fe000,               /* ldr lr, [pc, #0]  */
      0xe59ff000,               /* ldr pc, [pc, #0]  */
      0xffffffff,               /* <calculated LR    */
                                /*  goes here>       */
      0xffffffff                /* <calculated PC    */
                                /*  goes here>       */
    }, 4,
    3, 4,
    2, 2
  };
  branch_scenario_execute (&bs, fixture);
}

RELOCATOR_TESTCASE (bl_imm_a1_negative_should_be_rewritten)
{
  BranchScenario bs = {
    GUM_ARM_BL_IMM_A1,
    { 0xebffffff }, 1,          /* bl pc - 4         */
    {
      0xe59fe000,               /* ldr lr, [pc, #0]  */
      0xe59ff000,               /* ldr pc, [pc, #0]  */
      0xffffffff,               /* <calculated LR    */
                                /*  goes here>       */
      0xffffffff                /* <calculated PC    */
                                /*  goes here>       */
    }, 4,
    3, -4,
    2, 2
  };
  branch_scenario_execute (&bs, fixture);
}

RELOCATOR_TESTCASE (blx_imm_a2_positive_should_be_rewritten)
{
  BranchScenario bs = {
    GUM_ARM_BLX_IMM_A2,
    { 0xfb000001 }, 1,          /* blx pc + 6        */
    {
      0xe59fe000,               /* ldr lr, [pc, #0]  */
      0xe59ff000,               /* ldr pc, [pc, #0]  */
      0xffffffff,               /* <calculated LR    */
                                /*  goes here>       */
      0xffffffff                /* <calculated PC    */
                                /*  goes here>       */
    }, 4,
    3, 7,
    2, 2
  };
  branch_scenario_execute (&bs, fixture);
}

RELOCATOR_TESTCASE (blx_imm_a2_negative_should_be_rewritten)
{
  BranchScenario bs = {
    GUM_ARM_BLX_IMM_A2,
    { 0xfaffffff }, 1,          /* blx pc - 4        */
    {
      0xe59fe000,               /* ldr lr, [pc, #0]  */
      0xe59ff000,               /* ldr pc, [pc, #0]  */
      0xffffffff,               /* <calculated LR    */
                                /*  goes here>       */
      0xffffffff                /* <calculated PC    */
                                /*  goes here>       */
    }, 4,
    3, -3,
    2, 2
  };
  branch_scenario_execute (&bs, fixture);
}

static void
branch_scenario_execute (BranchScenario * bs,
                         TestArmRelocatorFixture * fixture)
{
  gsize i;
  guint32 calculated_pc;
  const GumArmInstruction * insn = NULL;

  for (i = 0; i != bs->input_length; i++)
    bs->input[i] = GUINT32_TO_LE (bs->input[i]);
  for (i = 0; i != bs->expected_output_length; i++)
    bs->expected_output[i] = GUINT32_TO_LE (bs->expected_output[i]);

  SETUP_RELOCATOR_WITH (bs->input);

  calculated_pc = fixture->rl.input_pc + 8 + bs->expected_pc_distance;
  *((guint32 *) (bs->expected_output + bs->pc_offset)) =
      GUINT32_TO_LE (calculated_pc);

  if (bs->lr_offset != -1)
  {
    guint32 calculated_lr;

    calculated_lr = (guint32) (fixture->aw.pc +
        (bs->expected_lr_distance * sizeof (guint32)));
    *((guint32 *) (bs->expected_output + bs->lr_offset)) =
        GUINT32_TO_LE (calculated_lr);
  }

  g_assert_cmpuint (gum_arm_relocator_read_one (&fixture->rl, &insn), ==, 4);
  g_assert_cmpint (insn->mnemonic, ==, bs->mnemonic);
  g_assert (gum_arm_relocator_write_one (&fixture->rl));
  gum_arm_writer_flush (&fixture->aw);
  g_assert_cmpint (memcmp (fixture->output, bs->expected_output,
      bs->expected_output_length * sizeof (guint32)), ==, 0);
}
