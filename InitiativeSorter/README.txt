A very basic table top role playing game turn tracker, which is capable of rolling initiatives, tracking turn orders, tracking hit points/round numbers, 
and which allows you to add characters manually or load them from files.

The following formats are supported as inputs to add creatures:
CreatureName Initiative InitiativeModifier hp:current/max

The order of Initiative and InitiativeModifier can be swapped, and modifiers are denoted using a + or -.
Both initiative and intiative modifiers are optional, but at least one must be included.
Hit point tracking can be enabled or disabled on a per-character basis by specifying hit points when it's added.

After all the characters are added, you can begin tracking by entering 'go'
Pressing 'Enter' advances to the next turn, though entering any command will prevent it from doing so.

Commands can be used to remove characters from the turn-order and adjust hit points.

'hp character_name amount' sets hit points directly - though a character's hit points cannot drop below 0 or above its maximum.
'hurt character_name amount' subtracts hit points from a character.
'heal character_name amonunt' adds hit points to a character, bounded by its maximum.
If a character is reduced to 0 hit points, the program will notify you.
If a character should be removed from the turn-order (usually because it ran out of HP), you can remove it with 'rm character_name' or 'remove character_name'

'quit' stops the program once it's tracking turns. If all characters are removed, that will also cause it to end.
