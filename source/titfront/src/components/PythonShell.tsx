/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Part of BlueTit Solver, licensed under Apache 2.0 with Commons Clause.
 * Commercial use, including SaaS, requires a separate license, see /LICENSE.md
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

import { FC, KeyboardEvent, useState, useEffect, useRef } from "react";
import {
  BiRadioCircle as CircleIcon,
  BiRadioCircleMarked as CircleFilledIcon,
} from "react-icons/bi";
import { DollarSign } from "react-feather";

import { PyError, usePython } from "./Python";
import { cn } from "../utils";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

type Command =
  | {
      input: string;
      status: "pending";
    }
  | {
      input: string;
      status: "success" | "failure";
      output: string;
    };

export const PythonShell: FC = () => {
  const [commands, setCommands] = useState<Command[]>([]);
  const inputRef = useRef<HTMLInputElement>(null);
  const runCode = usePython();

  const runCommand = (input: string) => {
    const cmdIndex = commands.length;
    if (input.length === 0) {
      setCommands([...commands, { input, output: "", status: "success" }]);
    } else {
      setCommands([...commands, { input, status: "pending" }]);
      runCode(input, (result) => {
        setCommands([
          ...commands.slice(0, cmdIndex),
          {
            input,
            status: result instanceof PyError ? "failure" : "success",
            output: (result as string).toString(),
          },
          ...commands.slice(cmdIndex + 1),
        ]);
      });
    }
  };

  let historyIndex = commands.length;
  const handleKeyDown = (e: KeyboardEvent<HTMLInputElement>) => {
    // Cmd+K clears the console on macOS. Let's do the same.
    if (e.key === "k" && (e.ctrlKey || e.metaKey)) {
      e.preventDefault();
      setCommands([]);
      return;
    }

    // Browse history.
    const target = e.target as HTMLInputElement;
    if (e.key === "ArrowUp") {
      e.preventDefault();
      if (historyIndex > 0) {
        target.value = commands[(historyIndex -= 1)].input;
      }
    } else if (e.key === "ArrowDown") {
      e.preventDefault();
      if (historyIndex < commands.length - 1) {
        target.value = commands[(historyIndex += 1)].input;
      } else {
        historyIndex = commands.length;
        target.value = "";
      }
    } else {
      historyIndex = commands.length;
    }

    // Run command.
    if (e.key === "Enter") {
      e.preventDefault();
      runCommand(target.value);
      target.value = "";
    }
  };

  // Scroll to bottom on new commands.
  useEffect(() => {
    inputRef.current?.scrollIntoView({ behavior: "smooth" });
  }, [commands]);

  const iconSize = 16;
  const columnCn = "grid grid-cols-[20px_auto]";
  return (
    <div
      className={cn(
        "h-full w-full flex flex-col overflow-auto",
        "font-mono select-text bg-gray-900"
      )}
      onClick={() => inputRef.current?.focus()}
    >
      {commands.map((cmd, index) => (
        <div key={index} className={columnCn}>
          <div className="flex items-center justify-center">
            {cmd.status === "pending" ? (
              <CircleIcon size={iconSize} />
            ) : (
              <CircleFilledIcon size={iconSize} />
            )}
          </div>
          <span className="break-all">{cmd.input}</span>
          <div />
          {cmd.status !== "pending" && cmd.input.length > 0 && (
            <span
              className={cn(
                "break-all",
                cmd.status === "failure" && "text-red-600"
              )}
            >
              {cmd.output}
            </span>
          )}
        </div>
      ))}
      <div key="input" className={columnCn}>
        <div className="flex items-center justify-center">
          <DollarSign size={iconSize} />
        </div>
        <div>
          <input
            type="text"
            className="w-full focus:outline-none"
            onKeyDown={handleKeyDown}
            ref={inputRef}
            autoFocus
          />
        </div>
      </div>
    </div>
  );
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
