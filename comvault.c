#include "comvault.h"
#include "bits.h"

#include "assert.h"
#include "string.h"

static CommandVault *commandVault;

typedef struct _InstructionsMemory {
  Instruction block;
  bit busy;
} InstructionsMemory;

static InstructionsMemory instructionsMemory[INSTRUCTIONS_MEMORY_SLOTS];

static void cleanInstructionsMemory(void)
{
  for(int i = 0; i < INSTRUCTIONS_MEMORY_SLOTS; i++)
  {
    instructionsMemory[i].busy = FALSE;
  }
}

static Instruction *allocateInstructionMemory(void)
{
  int i = 0;
  while((instructionsMemory[i].busy == TRUE) &&
        (i < INSTRUCTIONS_MEMORY_SLOTS))
  {
    i++;
  }
  if(i == INSTRUCTIONS_MEMORY_SLOTS)
  {
    return NULL;
  }
  instructionsMemory[i].busy = TRUE;
  return &instructionsMemory[i].block;
}

static bit freeInstructionMemory(Instruction *block)
{
  int i = 0;
  while((&instructionsMemory[i].block != block) &&
        (i < INSTRUCTIONS_MEMORY_SLOTS))
  {
    i++;
  }
  if(i == INSTRUCTIONS_MEMORY_SLOTS)
  {
    return FALSE;
  }
  instructionsMemory[i].busy = FALSE;
  return TRUE;
}

static bit cleanInstructionMemory(void)
{
  bit unusedBlockFound = FALSE;
  for(int i = 0; i < INSTRUCTIONS_MEMORY_SLOTS; i++)
  {
    if((instructionsMemory[i].busy == TRUE) &&
       ((instructionsMemory[i].block.condition == INSTRUCTION_STATUS_DONE) ||
        (instructionsMemory[i].block.condition == INSTRUCTION_STATUS_ERROR) ||
        (instructionsMemory[i].block.condition == INSTRUCTION_STATUS_BREAK)))
    {
      instructionsMemory[i].busy = FALSE;
      unusedBlockFound = TRUE;
    }
  }
  return unusedBlockFound;
}

void commandVault_init(CommandVault *cv)
{
  SANITY_CHECK(cv);
  commandVault = cv;
  commandVault->key = 0;
  
  commandVault->requests.totalInstructions = 0;
  cleanInstructionsMemory();
  
  commandVault->values.speedX = 0;
  commandVault->values.speedY = 0;
  commandVault->values.speedZL = 0;
  commandVault->values.speedZR = 0;
  
  commandVault->holdkeys.crossUp = 0;
  commandVault->holdkeys.crossDown = 0;
  commandVault->holdkeys.crossLeft = 0;
  commandVault->holdkeys.crossRight = 0;
  commandVault->holdkeys.buttonA = 0;
  commandVault->holdkeys.buttonB = 0;
  commandVault->holdkeys.buttonX = 0;
  commandVault->holdkeys.buttonY = 0;

  commandVault->outputs.endir12 = 0;
  commandVault->outputs.endir34 = 0;
  
  commandVault->status.ok = FALSE;
  commandVault->status.busy = FALSE;

}

void commandVault_lock(void)
{
  SANITY_CHECK(commandVault);
  while(commandVault->key);
  commandVault->key = 1;
}

void commandVault_unlock(void)
{
  SANITY_CHECK(commandVault);
  commandVault->key = 0;
}

int commandVault_locked(void)
{
  SANITY_CHECK(commandVault);
  return commandVault->key;
}

bit addInstruction(Instruction *ins)
{
  SANITY_CHECK(ins);
  Instruction *tmp = allocateInstructionMemory();
  SANITY_CHECK(tmp);
  memcpy(tmp, ins, sizeof(Instruction));
  tmp->next = commandVault->requests.instructions;
  commandVault->requests.instructions = tmp;
  if(commandVault->requests.totalInstructions++ < (MAX_SIMULTANEOUS_INSTRUCTIONS - 1))
  {
    return TRUE;
  }
  TRACE_DEBUG("Number of simultaneous instructions is close to maximum\n\r");
  TRACE_DEBUG("Performing instructions memory clean\n\r");
  if(cleanInstructionMemory())
  {
    return TRUE;
  }
  TRACE_DEBUG("Memory clean impossible\n\r");
  return FALSE;
}

bit removeInstructionByIdx(unsigned short idx)
{
  Instruction *head = commandVault->requests.instructions;
  Instruction *prev;
  if(head == NULL)
  {
    return FALSE;
  }
  if(head->next == NULL)
  {
    if(head->idx == idx)
    {
      commandVault->requests.instructions = NULL;
      freeInstructionMemory(head);
      commandVault->requests.totalInstructions--;
      return TRUE;
    }
  }
  if(head->idx == idx)
  {
    commandVault->requests.instructions = head->next;
    freeInstructionMemory(head);
    commandVault->requests.totalInstructions--;
    return TRUE;
  }
  do
  {
    prev = head;
    head = head->next;
    if(head->idx == idx)
    {
      prev->next = head->next;
      freeInstructionMemory(head);
      commandVault->requests.totalInstructions--;
      return TRUE;
    }
  }
  while(head->next);
  return FALSE;
}

Instruction *getInstruction(int pos)
{
  SANITY_CHECK(commandVault->requests.instructions);
  ASSERT((pos < commandVault->requests.totalInstructions), "Instruction index out of range");
  Instruction *head = commandVault->requests.instructions;
  int k = 0;
  do
  {
    if(k == pos)
    {
      return head;
    }
    head = head->next;
    k++;
  }
  while(head);  
  return NULL;
}

Instruction *getInstructionByIdx(unsigned short idx)
{
  Instruction *head = commandVault->requests.instructions;
  if(head == NULL)
  {
    return NULL;
  }
  do
  {
    if(head->idx == idx)
    {
      return head;
    }    
    head = head->next;
  }
  while(head);
  return NULL;
}
