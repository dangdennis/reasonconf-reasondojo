/* State declaration */
/* Action declaration */

[@react.component]
let make = (~greeting) =>
  <>
    <div> {ReasonReact.string(greeting)} </div>
    <Reant.Button _type="primary">
      {ReasonReact.string("Pretty Button")}
    </Reant.Button>

  </>;
