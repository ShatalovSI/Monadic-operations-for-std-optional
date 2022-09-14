# Monadic operations for std::optional

`std::optional` is an important vocabulary type in C++17. Some uses of it are verbose and would benefit from operations which allow functional composition. I propose adding transform, and_then, and or_else member functions to `std::optional` to support this monadic style of programming.

## Motivation

`std::optional` aims to be a "vocabulary type", i.e. the canonical type to represent some programming concept. As such, `std::optional` will become widely used to represent an object which may or may not contain a value. Unfortunately, chaining together many computations which may or may not produce a value can be verbose, as empty-checking code will be mixed in with the actual programming logic. As an example, the following code automatically extracts cats from images and makes them more cute:

    image get_cute_cat (const image& img) {
       return add_rainbow(             
        make_smaller(               
          make_eyes_sparkle(                 
            add_bow_tie(                   
              crop_to_cat(img))));}
              
But there’s a problem. What if there’s not a cat in the picture? What if there’s no good place to add a bow tie? What if it has its back turned and we can’t make its eyes sparkle? Some of these operations could fail.

One option would be to throw exceptions on failure. However, there are many code bases which do not use exceptions for a variety of reasons. There’s also the possibility that we’re going to get lots of pictures without cats in them, in which case we’d be using exceptions for control flow. This is commonly seen as bad practice, and has an item warning against it in the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#e3-use-exceptions-for-error-handling-only).

Another option would be to make those operations which could fail return a `std::optional`:

    std::optional<image> get_cute_cat (const image& img) {    
      auto cropped = crop_to_cat(img);    
      if (!cropped) {      
        return std::nullopt;    
      }
      
      auto with_tie = add_bow_tie(*cropped);    
      if (!with_tie) {      
        return std::nullopt;    
      }
      
      auto with_sparkles = make_eyes_sparkle(*with_tie);    
      if (!with_sparkles) {      
        return std::nullopt;    
      }    
      return add_rainbow(make_smaller(*with_sparkles));
    }
      
Our code now has a lot of boilerplate to deal with the case where a step fails. Not only does this increase the noise and cognitive load of the function, but if we forget to put in a check, then suddenly we’re down the hole of undefined behaviour if we `*empty_optional`.

Another possibility would be to call `.value()` on the optionals and let the exception be thrown and caught like so:

    std::optional<image> get_cute_cat (const image& img) {    
      try {        
        auto cropped = crop_to_cat(img);        
        auto with_tie = add_bow_tie(cropped.value());        
        auto with_sparkles = make_eyes_sparkle(with_tie.value());        
        return add_rainbow(make_smaller(with_sparkles.value()));    
      catch (std::bad_optional_access& e) {        
        return std::nullopt;    
      }
    }
    
Again, this is using exceptions for control flow. There must be a better way.

## Proposed solution

This paper proposes adding additional member functions to `std::optional` in order to push the handling of empty states off to the side. The proposed additions are `transform`, `and_then` and `or_else`. Using these new functions, the code above becomes this:

    std::optional<image> get_cute_cat (const image& img) {    
      return crop_to_cat(img)           
            .and_then(add_bow_tie)           
            .and_then(make_eyes_sparkle)           
            .transform(make_smaller)           
            .transform(add_rainbow);
    }

We’ve successfully got rid of all the manual checking. We’ve even improved on the clarity of the non-optional example, which needed to either be read inside-out or split into multiple declarations.
