# Setup

1. Add the following line to your <PROJECT_NAME>.h file:

```
DECLARE_LOG_CATEGORY_EXTERN(Log<PROJECT_NAME>, Log, All)
```

2. Add the following line to your <PROJECT_NAME>.cpp file:
```
DEFINE_LOG_CATEGORY(Log<PROJECT_NAME>)
```

3. Replace ALL instances of `<PROJECT_NAME>` with the the name of your project (in PascalCase) and all `<PROJECT_NAME_UPPER>` with the name in all caps and no spaces

4. Add the following plugins to your project:
- `Enhanced Input`
- `Gameplay Abilities`
- `Game Features`
- `Modular Gameplay`
- `Ability System Game Feature Actions`

5. Add the following to your dependency lists in your `PROJECT_NAME.build.cs` file:

```

PublicDependencyModuleNames.AddRange(new string[] { "EnhancedInput" });

PrivateDependencyModuleNames.AddRange(new string[] {"ModularGameplay", "GameFeatures", "GameplayAbilities", "GameplayTags", "GameplayTasks"  });

```


6. Replace all instances of `PROJECT_NAME` in file names to the name of your project(in PascalCase) 

7. Finish setting up the [GameplayAbilitySystem](https://www.youtube.com/watch?v=LxT8Fc2ejgI) (if not done already)

8. Update input settings to use enhanced input classes


![Input Settings](/InputSettings.jpg)

9. Edit the level blueprint and add code to enable your new plugins


![Event Level Graph](/LevelEventGraph.jpg)
