#pragma once

#include <mbgl/style/transitioning_property.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/style/property_value.hpp>
#include <mbgl/style/property_evaluator.hpp>
#include <mbgl/style/property_evaluation_parameters.hpp>
#include <mbgl/style/transition_options.hpp>
#include <mbgl/style/cascade_parameters.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/interpolate.hpp>
#include <mbgl/util/indexed_tuple.hpp>
#include <mbgl/util/ignore.hpp>

#include <memory>

namespace mbgl {
namespace style {

template <class Value>
class CascadingLightProperty {
public:
    bool isUndefined() const {
        return bool(value);
    }

    const Value& get() const {
        return value;
    }

    void set(const Value& value_) {
        value = value_;
    }

    const TransitionOptions& getTransition() const {
        if (bool(transition)) {
            return transition;
        } else {
            static const TransitionOptions staticValue{};
            return staticValue;
            // TODO write this in one line probably
        }
    }

    void setTransition(const TransitionOptions& transition_) {
        transition = transition_;
    }

    template <class TransitioningProperty>
    TransitioningProperty cascade(const CascadeParameters& params,
                                  TransitioningProperty prior) const {
        TransitionOptions transition_;
        Value value_;

        if (bool(value)) {
            value_ = value;
        }

        if (bool(transition)) {
            transition_ = transition;
        }

        return TransitioningProperty(std::move(value_), std::move(prior),
                                     transition_.reverseMerge(params.transition), params.now);
    }

private:
    Value value;
    TransitionOptions transition;
};

template <class T>
class LightProperty {
public:
    using ValueType = PropertyValue<T>;
    using CascadingType = CascadingLightProperty<ValueType>;
    using UnevaluatedType = TransitioningProperty<ValueType>;
    using EvaluatorType = PropertyEvaluator<T>;
    using EvaluatedType = T;
};

template <class... Ps>
class LightProperties {
public:
    using Properties = TypeList<Ps...>;

    using EvaluatedTypes = TypeList<typename Ps::EvaluatedType...>;
    using UnevaluatedTypes = TypeList<typename Ps::UnevaluatedType...>;
    using CascadingTypes = TypeList<typename Ps::CascadingType...>;

    template <class TypeList>
    using Tuple = IndexedTuple<Properties, TypeList>;

    class Evaluated : public Tuple<EvaluatedTypes> {
    public:
        using Tuple<EvaluatedTypes>::Tuple;
    };

    class Unevaluated : public Tuple<UnevaluatedTypes> {
    public:
        using Tuple<UnevaluatedTypes>::Tuple;
    };

    class Cascading : public Tuple<CascadingTypes> {
    public:
        using Tuple<CascadingTypes>::Tuple;
    };

    template <class P>
    auto get() const {
        return cascading.template get<P>().get();
    }

    template <class P>
    void set(const typename P::ValueType& value) {
        cascading.template get<P>().set(value);
    }

    template <class P>
    TransitionOptions getTransition() {
        return cascading.template get<P>().getTransition();
    }

    template <class P>
    void setTransition(const TransitionOptions& value) {
        cascading.template get<P>().setTransition(value);
    }

    void cascade(const CascadeParameters& parameters) {
        unevaluated = Unevaluated{ cascading.template get<Ps>().cascade(
            parameters, std::move(unevaluated.template get<Ps>()))... };
    }

    template <class P>
    auto evaluate(const PropertyEvaluationParameters& parameters) {
        using Evaluator = typename P::EvaluatorType;
        return unevaluated.template get<P>().evaluate(Evaluator(parameters, P::defaultValue()),
                                                      parameters.now);
    }

    void evaluate(const PropertyEvaluationParameters& parameters) {
        evaluated = Evaluated{ evaluate<Ps>(parameters)... };
    }

    bool hasTransition() const {
        bool result = false;
        util::ignore({ result |= unevaluated.template get<Ps>().hasTransition()... });
        return result;
    }

    Cascading cascading;
    Unevaluated unevaluated;
    Evaluated evaluated;
};

struct LightAnchor : LightProperty<LightAnchorType> {
    static LightAnchorType defaultValue() {
        return LightAnchorType::Viewport;
    }
};
struct LightPosition : LightProperty<Position> {
    static Position defaultValue() {
        std::array<float, 3> default_ = { { 1.15, 210, 30 } };
        return Position{ { default_ } };
    }
};
struct LightColor : LightProperty<Color> {
    static Color defaultValue() {
        return Color::white();
    }
};
struct LightIntensity : LightProperty<float> {
    static float defaultValue() {
        return 0.5;
    }
};

class Light : public LightProperties<LightAnchor, LightPosition, LightColor, LightIntensity> {};

} // namespace style
} // namespace mbgl
