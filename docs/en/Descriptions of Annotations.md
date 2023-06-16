# Annotations of Modern Stagger Lock Animations

  <br/>

## Next Stagger Window Annotations

---

### Trigger Transition Window Open:

- MSL_SmallStaggerTriggerOpen
- MSL_MediumStaggerTriggerOpen
- MSL_LargeStaggerTriggerOpen
- MSL_LargestStaggerTriggerOpen
  <br/>

### Initiate Transition Window Open:

- MSL_SmallStaggerInitiateOpen
- MSL_MediumStaggerInitiateOpen
- MSL_LargeStaggerInitiateOpen
- MSL_LargestStaggerInitiateOpen
  <br/>

### Transition Window Close:

- MSL_SmallStaggerWinClose
- MSL_MediumStaggerWinClose
- MSL_LargeStaggerWinClose
- MSL_LargestStaggerWinClose
  <br/>

### More About Trigger/Initiate Transition:

Trigger transition could only be actived after the `XXXTriggerOpen` event has fired and before the `XXXWinClose` event has been fired, a `staggerStart` event sending between this interval would trigger the next stagger animation instantly without any delay. However, trigger transition still not guaranteed to be fired 100% despite you annotate the `XXXTriggerOpen` at 0.0 local time.

Initiate transition was actived for all the time before `XXXWinClose` event has been fired, a `staggerStart` event sending before `XXXWinClose` event could trigger the next stagger animation, but the transtion would be delayed until `XXXInitiateOpen` is fired. Initiate transition could guaranteed to be fired 100% as long as your annotate the specify `XXXInitiateOpen` event.

It's recommended to use trigger window to transit to the same or lower level stagger animation, while using initiate window to transit to higher level stagger animation.

You should not annotate both TriggerOpen and InitiateOpen for a same stagger level inside single stagger animation file. For example, do not have both `MSL_SmallStaggerTriggerOpen` and `MSL_SmallStaggerInitiateOpen` existed in the same animation file!

---

<br/> <br/>

## Recovery Window Annotations

---

### Window Open:

- MSL_RecovWinOpen

### Window Close:

- MSL_RecovWinClose

---
